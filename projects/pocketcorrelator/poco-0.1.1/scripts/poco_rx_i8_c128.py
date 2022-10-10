#!/usr/bin/python
"""
Record data to Miriad files from the CasperN Pocket Correlator.
Author: Aaron Parsons
Modified: Jason Manley 
Date: 2008/02/10

Revisions:
2009-04-09:	GSF	corrected the packet->miriad path to write the packet data correctly to .uv file

2008-06-24:	GSF	poco_rx.py is based on the cn_rx.py script to handle PoCo packets and
				record them to MIRIAD UV files
				_process_packets() and apply_data() have the significant changes

2008-03-28: JRM channel agility bugfix

2008-02-08: JRM circular buffer for out-of-order packet reception
                selective single/multiple miriad file write
                new config file format
                stored integrations now correct from first one (free with circular buffer)
                receive packets now in seperate thread
                clean exit (closes miriad files after ctl-c)

2008-01-30: JRM UDP-only source
                New corr config file for params
                option to only record one file, then terminate
                bandpass recording fixes
                

"""

import poco.c2m, poco.packet, poco.cn_conf
import time, numpy, os, threading, subprocess, sys

class C2M_RX:
    """An interface for recording UDP correlator transmissions to Miriad 
    UV files.  Subclasses twisted's event-driven DatagramProtocol for handling 
    UDP sockets."""
    def __init__(self, loc, ants, n_chans=2048, n_ants=8, acc_len=8192, 
        adc_clk=0.600, xeng_acc_len=128, n_stokes=4, ddc_mix_freq=0.25, 
        ddc_decimation=4, clk_per_sync=(2**18),t_per_file=3600, dbfile=None, 
        eq_polys=[1], single_capture=False, verbose=False,port=8888, buffer_size=8192,
        tx_pkt_size=1045, tx_pkt_header=21, nchan_per_bram=1024,
        pkts_per_bram=4, brams_per_pkt=1, nbrams=16, design_name='poco4_c1024',
        eq_blocks=1,zero_ground=0):
        """t_per_file  = The number of seconds of data written to each Miriad file.
        dbfile      = An optional database file useful for plotting and web 
                      interfaces."""

        self.mrec = poco.c2m.MiriadRecorder(nchans=n_chans, ants=ants, 
            acc_len=acc_len, adc_clk_rate=adc_clk, 
            comp_chans=n_chans, clk_per_sync=clk_per_sync, 
            n_stokes=n_stokes, ddc_mix_freq=ddc_mix_freq, 
            ddc_decimation=ddc_decimation, location=loc)

        self.t_per_file = t_per_file
        self.n_ants = len(ants)
        self.n_stokes = n_stokes
        self.n_xeng = 1
        self.stokes = ['xy','yx','xx','yy'][:n_stokes]
        self.n_chans = n_chans
        self.acc_len = acc_len
        self.adc_clk_rate = adc_clk
        self.xeng_acc_len = xeng_acc_len
        self.ddc_mix_freq = ddc_mix_freq
        self.ddc_decimation = ddc_decimation
        self.eq_polys = eq_polys
        self.bl_order = poco.sim_cn_data.get_bl_order_8()
        self.n_bls = len(self.bl_order)
        self.data = numpy.zeros((self.n_chans, self.n_bls*self.n_stokes*2),dtype=numpy.int32)
        self.pkt = poco.packet.CasperN_Packet(endian='>', type='poco')
        self.start_t = 0
        self.gain = acc_len
        self._quit=False
        self.single_capture=single_capture
        self.verbose=verbose
        self.port = port
        self.buffer_size=buffer_size
        self.n_int_buffer=4
        self.timestamp_scale_factor = 1
        self.timestamp_rounding = 2 
        self.read_ahead_allow = 0.20
        self.ndb = None
        self.tx_pkt_size = tx_pkt_size
        self.tx_pkt_header = tx_pkt_header
        self.nchan_per_bram = nchan_per_bram
        self.pkts_per_bram = pkts_per_bram
        self.brams_per_pkt = brams_per_pkt
        self.nbrams = nbrams
        self.design_name = design_name
        self.eq_blocks = eq_blocks
        self.zero_ground = zero_ground
        if self.pkts_per_bram == 1 and self.brams_per_pkt > 1: self.pkts_per_int = self.nbrams/self.brams_per_pkt
        else: self.pkts_per_int = self.nbrams * self.pkts_per_bram

    def open_file(self):
        """Start a Miriad file called 'zen.uv.tmp' in current directory."""
        print 'Starting file: zen.uv.tmp'
        if os.path.exists('zen.uv.tmp'): os.system('rm -rf zen.uv.tmp')
        self.mrec.open_uv('zen.uv.tmp')
        self.start_t = time.time()

    def close_file(self):
        """Close 'zen.uv.tmp' and rename it to 'poco.<Julian.Date>.uv' 
        according to its start time."""
        jd_start_t = self.mrec.conv_time(self.start_t)
        filename = 'poco.%7.5f.uv' % (jd_start_t)
        print 'Renaming zen.uv.tmp to %s' % (filename)
        self.mrec.close_uv(filename)
        self.start_t = 0

    def set_bandpass(self):
        """Setup the bandpass to be saved to this Miriad file."""
        bps = []
        print 'Recording Bandpass to file...',
        for i in range(self.eq_blocks):
            bp = numpy.polyval(self.eq_polys[i], numpy.arange(self.nchan_per_bram))
            #print 'BP[%i]:'%i,bp
            bp = numpy.clip(bp, 0, 2**17 - 1)
            bps.append(bp)
        bps = numpy.concatenate(bps, axis=0)
        bps = bps.astype(numpy.complex)
        bps = 1.0/bps
        bps.shape=(self.n_ants,self.n_chans)
        self.mrec.set_bandpass(bps)

    def process_integration(self, data, timestamp):
        """
        data is a 2D array of shape 
        n_ants x (2 *n_stokes*n_bls*n_chans per x_eng)
        """
        self.data = data
        self.file_data(timestamp)
        if (time.time() - self.start_t) > self.t_per_file and self.start_t>0:
            self.close_file()
            if not self.single_capture:
                self.open_file()
            else:
                print 'Grabbed a single file... exiting.'
                self._quit=True

    def file_data(self, t):
        #print 'Recording data for timestamp %f' % t           
        # Write spectrum for each stokes, baseline to file.
        #stokes/pol in poco are the same thing, so do not write stokes information
        poco_stokes = ['xx']
        for s,pol in enumerate(poco_stokes):
            for b, bl in enumerate(self.bl_order):
                i, j = poco.sim_cn_data.bl2ij(bl)
                d = self.data[:,b,:].astype(numpy.float)
                if i > j:
                     d = numpy.conjugate(d)
                     i,j = j,i
                #print sum(d),
                #scale the data - fixed-point format is 32_6 by default
                #d /= self.gain
                #turn the two independant values into a single complex number:
                dfloat = d[...,0] - d[...,1]*1j
                # Write to MIRIAD file
                self.mrec.record(t, i, j, pol, dfloat)
                # Optionally write to database file as well
                if not self.ndb is None:
                    self.ndb.write('%i-%i,%s' % (i, j, pol), dfloat)
        # Clear the data buffer
        self.data *= 0

    def start(self):
        """Starts reception "process_packets" in a new thread."""
        self._rx_thread = threading.Thread(target=self._process_packets,args=())
        self._rx_thread.setDaemon(1)
        print 'Beginning RX thread...'
        self._rx_thread.start()

    def stop(self):
        print 'Stopping...'
        self._quit = True
        self.close_file()
        self._rx_thread.join()
    
    def _auto_parse(self, data):
        combine_data = numpy.zeros(len(data)/4)
        for d in range(len(data)/4):
            d0 = data[d*4]
            d1 = data[d*4+1]*256
            d2 = data[d*4+2]*65536
            d3 = data[d*4+3]*16777216
            df = d0 + d1 + d2 + d3 
            combine_data[d] = df
        return combine_data
    
    def _cross_parse(self, data):
        combine_data = numpy.zeros(len(data)/4)
        for d in range(len(data)/4):
            if data[d*4+3] > 127:
                d0 = -1*(255-data[d*4])
                d1 = -1*(255-data[d*4+1])*256
                d2 = -1*(255-data[d*4+2])*65536
                d3 = -1*(255-data[d*4+3])*8388608
            else:
                d0 = data[d*4]
                d1 = data[d*4+1]*256
                d2 = data[d*4+2]*65536
                d3 = data[d*4+3]*8388608
            df = d0 + d1 + d2 + d3 
            combine_data[d] = df
        return combine_data

    def _process_packets(self):
        """modified for poco"""
        correlator = poco.packet.CasperN_RX_correlator(port=self.port, buffer_size=self.buffer_size)
        pkt = poco.packet.CasperN_Packet(endian='>', type='poco')

        #total_p_size is the number of words per x engine of one integration, for poco there is only 1 xeng
        #so this is the total number of words for an integration
        #total_p_size = 2 * self.n_stokes * self.n_bls * self.n_chans/self.n_ants
        n_integrations = self.n_ants + 2*(self.n_bls - self.n_ants)
        total_p_size = self.n_chans * n_integrations 
        total_bytes_per_xeng = (total_p_size*self.n_xeng)*pkt.word_size
        #print 'total_p_size', total_p_size
        #print 'total_bytes_per_xeng', total_bytes_per_xeng
        word_size = pkt.word_size
        #print 'word_size',word_size

        timestamps=[0]*n_integrations
        next_write_location = 0
        last_timestamp=0
        last_int_time=0
        integration_counter = 0

        auto_offsets = [0,1,2,3]
        cross_real_offsets = [4,6,8,10,12,14,16,18,20,22,24,26,28,30]
        cross_imag_offsets = [5,7,9,11,13,15,17,19,21,23,25,27,29,31]
        current_offset = 0
        full_int_data = numpy.zeros([self.n_chans,self.n_bls,2])
        baseline_number = 0
        baseline_data = numpy.zeros([self.n_chans]) 
        print 'Expecting total dump size from each x engine: %i bytes'%total_bytes_per_xeng

        #get a starting timestamp:
        p = pkt.unpack(correlator.get_a_packet())
        last_offset = p['offset']
        while last_offset < self.pkts_per_int - 1:
            p = pkt.unpack(correlator.get_a_packet())
            last_offset = p['offset']
        if p == None:
            print 'ERR: couldnt get a good packet to determine starting time'
            exit()
        else:
            if p['offset'] == 0:
                last_timestamp = p['time']
            elif last_timestamp == 0:
                last_timestamp = p['time']
            max_time_allow = self.read_ahead_allow + p['time']

        try:
            while not self._quit:
                # Get a packet and unpack
                p = pkt.unpack(correlator.get_a_packet())
                #debug_time = time.time()
                if p['offset'] == 0: last_timestamp = p['time']
                if not p == None:
                    if self.verbose:
                        print ' Got offset %i for X engine %i, timestamp %.2f ' \
                            'with labelled length %i. %i numbers.' %(p['offset'], 
                            p['xeng'],p['time'],p['data_size'], len(p['data']))

                    if p['offset'] in auto_offsets:
                        combine_data = self._auto_parse(p['data'])
                        full_int_data[:,baseline_number,0] = combine_data[::2]
                        full_int_data[:,baseline_number,1] = numpy.zeros_like(combine_data[::2])
                        baseline_number += 1
                        full_int_data[:,baseline_number,0] = combine_data[1::2]
                        full_int_data[:,baseline_number,1] = numpy.zeros_like(combine_data[1::2])
                        baseline_number += 1
                    elif p['offset'] in cross_real_offsets:
                        combine_data = self._cross_parse(p['data'])
                        full_int_data[:,baseline_number,0] = combine_data[::2]
                        full_int_data[:,baseline_number+1,0] = combine_data[1::2]
                    elif p['offset'] in cross_imag_offsets:
                        combine_data = self._cross_parse(p['data'])
                        full_int_data[:,baseline_number,1] = combine_data[::2]
                        full_int_data[:,baseline_number+1,1] = combine_data[1::2]
                        baseline_number += 2
                    
                    #Check if we need to save this integration:
                    if p['offset'] == self.pkts_per_int - 1:
                        timestamp = p['time']
                        #print 'Saving integration %i after time %2.2f'%(timestamp_index,timestamp-last_timestamp)
                        #print '\n Saving Integration'
                        #print numpy.sum(full_int_data),timestamp 
                        full_int_data = numpy.flipud(full_int_data)
                        if self.zero_ground == 1: full_int_data[0] = 0
                        self.process_integration(full_int_data,timestamp)
                        integration_counter+=1
                        #print 'time offset from last integration(seconds): %f'%(timestamp-last_int_time)
                        #print timestamp-last_int_time
                        print '.',
                        if integration_counter is 10:
                            print 'Processed 10 integrations',timestamp
                            integration_counter = 0
                        full_int_data = numpy.zeros([self.n_chans,self.n_bls,2])
                        baseline_number = 0
                        #reset this data:
                        last_int_time = timestamp
                        timestamp = last_timestamp
                else:
                    print 'Packet unpack error'
        except(KeyboardInterrupt):
            print 'Keyboard interrupt...',
            self.stop()   


#  ____            _       _     ___       _             __                
# / ___|  ___ _ __(_)_ __ | |_  |_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___ 
# \___ \ / __| '__| | '_ \| __|  | || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
#  ___) | (__| |  | | |_) | |_   | || | | | ||  __/ |  |  _| (_| | (_|  __/
# |____/ \___|_|  |_| .__/ \__| |___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
#                   |_|                                                    


if __name__ == '__main__':
    import sys
    from optparse import OptionParser
    
    p = OptionParser()
    p.set_usage('cn_rx.py [options] CONFIG_FILE')
    p.set_description(__doc__)
    
    p.add_option('-v', '--verbose', dest='verbose', action='store_true',
        help='Be verbose (good for debugging)')
    
    p.add_option('-s', '--single_capture', dest='single_capture', action='store_true',
        help='Only do a single capture (one MIRIAD file). Default: continuously create new MIRIAD files.')
    opts, args = p.parse_args(sys.argv[1:])
    
    if args==[]:
        print 'Please specify a configuration file! \nExiting.'
        exit()
    
    config = poco.cn_conf.CorrConf(args[0], type='poco')
    config_status = config.read_all()
    
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()
    
    ANTPOS = [config['antennas']['pos'][a] for a in range(config['n_ants'])]
    ants = [ANTPOS[a] for a in config['antennas']['order']] 
    LOC = config['antennas']['location']
    ADC_CLK = config['adc_clk']
    UDP_PORT = config['udp_port']
    X_PER_FPGA = config['x_per_fpga']
    DDC_MIX_FREQ = config['ddc_mix_freq']
    DDC_DECIMATION = config['ddc_decimation']
    N_CHANS = config['n_chans']
    N_STOKES = config['n_stokes']
    ACC_LEN = config['acc_len']
    N_ANTS = config['n_ants']
    T_PER_FILE = config['t_per_file']
    RX_BUFFER_SIZE = config['rx_buffer_size']
    
    DEFAULT_EQ=config['eq']['eq_polys']
    
    TX_PKT_SIZE = config['tx_pkt_size']
    TX_PKT_HEADER = config['tx_pkt_header_size']
    NCHAN_PER_BRAM = config['nchan_per_bram']
    PKTS_PER_BRAM = config['pkts_per_bram']
    BRAMS_PER_PKT = config['brams_per_pkt']
    NBRAMS = config['nbrams']
    DESIGN_NAME = config['design_name']
    EQ_BLOCKS = config['eq_blocks']
    ZERO_GROUND = config['zero_ground']
    
    n_bls = N_ANTS * (N_ANTS+1.0)/2
    
	#we want int_time = prms['acc_len'] / (bandwidth * 1e9) in c2m.py
	#but it is int_time = prms['clk_per_sync'] / (bandwidth * 1e9) * prms['acc_len']
    CLK_PER_SYNC = 1
    #XENG_ACC_LEN not used in poco
    XENG_ACC_LEN = 128
    
    print 'Listening on port %i' %UDP_PORT
    int_size = 2 * N_STOKES * n_bls * N_CHANS
    print 'Expecting integration sizes of %i vectors, or %i bytes'%(int_size, int_size*4)
    
    udp_rx = C2M_RX(loc=LOC, n_chans=N_CHANS, ants=ants, 
        acc_len=ACC_LEN, adc_clk=ADC_CLK, xeng_acc_len=XENG_ACC_LEN, 
        n_stokes=N_STOKES, ddc_mix_freq=DDC_MIX_FREQ, verbose=opts.verbose,
        ddc_decimation=DDC_DECIMATION, t_per_file=T_PER_FILE,
        eq_polys=DEFAULT_EQ,single_capture=opts.single_capture,
        port=UDP_PORT, buffer_size=RX_BUFFER_SIZE, clk_per_sync=CLK_PER_SYNC,
        tx_pkt_size=TX_PKT_SIZE,tx_pkt_header=TX_PKT_HEADER,nchan_per_bram=NCHAN_PER_BRAM,
        pkts_per_bram=PKTS_PER_BRAM,brams_per_pkt=BRAMS_PER_PKT,nbrams=NBRAMS,
        design_name=DESIGN_NAME,eq_blocks=EQ_BLOCKS,zero_ground=ZERO_GROUND)

    udp_rx.set_bandpass()
    udp_rx.open_file()

    udp_rx.start()

    try:
        while not udp_rx._quit: 
            time.sleep(1)
    except(KeyboardInterrupt):
            udp_rx.stop()
    print '...Exiting.'
    

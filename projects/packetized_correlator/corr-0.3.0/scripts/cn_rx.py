#!/usr/bin/python
"""
Record data to Miriad files from the CasperN correlator.
Author: Aaron Parsons
Modified: Jason Manley 
Date: 2008/02/10

Revisions:
2008-09-10: JRM added support for different n_xeng/n_ants combinations

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

import corr.c2m, corr.packet, corr.sim_cn_data, corr.plotdb, corr.borph, corr.cn_conf
import time, numpy, os, threading, subprocess


class C2M_RX:
    """An interface for recording UDP correlator transmissions to Miriad 
    UV files.  Subclasses twisted's event-driven DatagramProtocol for handling 
    UDP sockets."""
    def __init__(self, loc, ants, n_chans=2048, n_xeng=8, acc_len=8192, 
        adc_clk=0.600, xeng_acc_len=128, n_stokes=4, ddc_mix_freq=0.25, 
        ddc_decimation=4, clk_per_sync=(2**18),t_per_file=3600, dbfile=None, 
        eq_polys=[1], single_capture=False, verbose=False,port=8888, buffer_size=8192):
        """t_per_file  = The number of seconds of data written to each Miriad file.
        dbfile      = An optional database file useful for plotting and web 
                      interfaces."""
        print 'Location: ',loc
        self.mrec = corr.c2m.MiriadRecorder(nchans=n_chans, ants=ants, 
            acc_len=acc_len, adc_clk_rate=adc_clk, 
            comp_chans=n_chans, clk_per_sync=clk_per_sync, 
            n_stokes=n_stokes, ddc_mix_freq=ddc_mix_freq, 
            ddc_decimation=ddc_decimation, location=loc)

        self.t_per_file = t_per_file
        self.n_ants = len(ants)
        self.n_stokes = n_stokes
        self.n_xeng = n_xeng
        self.stokes = ['xy','yx','xx','yy'][:n_stokes]
        self.n_chans = n_chans
        self.acc_len = acc_len
        self.adc_clk_rate = adc_clk
        self.xeng_acc_len = xeng_acc_len
        self.ddc_mix_freq = ddc_mix_freq
        self.ddc_decimation = ddc_decimation
        self.eq_polys = eq_polys
        self.bl_order = corr.sim_cn_data.get_bl_order(self.n_ants)
        self.n_bls = len(self.bl_order)
        self.data = numpy.zeros((self.n_chans, self.n_bls*self.n_stokes*2),dtype=numpy.int32)
        self.pkt = corr.packet.CasperN_Packet(endian='>')
        self.start_t = 0
        self.gain = xeng_acc_len * acc_len
        self._quit=False
        self.single_capture=single_capture
        self.verbose=verbose
        self.port = port
        self.buffer_size=buffer_size
        self.n_int_buffer=4
        self.timestamp_scale_factor = 1./1000000
        self.timestamp_rounding = 0
        self.int_time = float(self.acc_len)*self.xeng_acc_len*self.n_chans/(self.adc_clk_rate*1000000000.0/self.ddc_decimation)
        self.read_ahead_allow = int((2*self.int_time)/self.timestamp_scale_factor)

        # Optional database file for plotting and web-interfacing
        self.ndb = None
        if not dbfile is None: 
            print "Starting dbfile %s"%dbfile
            self.ndb = corr.plotdb.PlotDB(dbfile,mode='c')
            self.ndb.WriteInfoFile(self.adc_clk_rate,self.ddc_decimation,self.ddc_mix_freq,self.n_chans,self.n_ants,self.n_stokes,self.xeng_acc_len,self.acc_len, self.eq_polys)

    def open_file(self):
        """Start a Miriad file called 'zen.uv.tmp' in current directory."""
        print 'Starting file: zen.uv.tmp'
        if os.path.exists('zen.uv.tmp'): os.system('rm -rf zen.uv.tmp')
        self.mrec.open_uv('zen.uv.tmp')
        self.start_t = time.time()

    def close_file(self):
        """Close 'zen.uv.tmp' and rename it to 'zen.<Julian.Date>.uv' 
        according to its start time."""
        jd_start_t = self.mrec.conv_time(self.start_t)
        filename = 'zen.%7.5f.uv' % (jd_start_t)
        print 'Renaming zen.uv.tmp to %s' % (filename)
        self.mrec.close_uv(filename)
        self.start_t = 0

    def set_bandpass(self):
        """Setup the bandpass to be saved to this Miriad file."""
        bps = []
        for i in range(self.n_ants):
            bp = numpy.polyval(self.eq_polys[i], numpy.arange(self.n_chans))
            print 'BP[%i]:'%i,bp
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
        n_xeng x (2 *n_stokes*n_bls*n_chans per x_eng)
        """
        for x in range(self.n_xeng):
            self.apply_data(x,data[x])
        self.file_data(timestamp)
        
        if (time.time() - self.start_t) > self.t_per_file and self.start_t>0:
            self.close_file()
            if not self.single_capture:
                self.open_file()
            else:
                print 'Grabbed a single file... exiting.'
                self._quit=True

    def apply_data(self, xeng, data):
        """
        xeng is the number of this x engine
        data is a list of all data for this x engine
        """
        for c in range(self.n_chans / self.n_xeng):
            chan = c * self.n_xeng + xeng
            #fix the spectrum (move second half to first half):
            if chan < (self.n_chans/2):
                offset1 = (c+(self.n_chans/self.n_xeng/2)) * self.n_bls * self.n_stokes * 2 
                offset2 = (c+(self.n_chans/self.n_xeng/2)+1) * self.n_bls * self.n_stokes * 2                
            else:
                offset1 = (c-(self.n_chans/self.n_xeng/2)) * self.n_bls * self.n_stokes * 2
                offset2 = (c-(self.n_chans/self.n_xeng/2)+1) * self.n_bls * self.n_stokes * 2

            try: self.data[chan] = data[offset1:offset2]
            except(ValueError): 
                print 'There was an error processing the last data chunk' \
                        ' in range %i:%i'%(offset1,offset2)
                
    def file_data(self, t):
        #print 'Recording data for timestamp %f' % t           
        self.data.shape = (self.n_chans, self.n_bls, self.n_stokes, 2)
        # Write spectrum for each stokes, baseline to file.
        for s,pol in enumerate(self.stokes):
            for b, bl in enumerate(self.bl_order):
                i, j = corr.sim_cn_data.bl2ij(bl)
                d = self.data[:,b,s,:].astype(numpy.float)
                #scale the data - fixed-point format is 32_6 by default
                d /= self.gain
                #turn the two independant values into a single complex number:
                dfloat = d[...,0] - d[...,1]*1j
                # Write to MIRIAD file
                self.mrec.record(t, i, j, pol, dfloat)
                # Optionally write to database file as well
                if not self.ndb is None:
                    self.ndb.write('%i-%i,%s' % (i, j, pol), dfloat)
        # Clear the data buffer
        self.data.shape = (self.n_chans, self.n_bls * self.n_stokes * 2)
        self.data *= 0

    def start(self):
        """Starts reception "process_packets" in a new thread."""
        self._rx_thread = threading.Thread(target=self._process_packets,args=())
        #self._rx_thread.setDaemon(1)
        print 'Beginning RX thread...'
        self._rx_thread.start()

    def stop(self):
        print 'Stopping...'
        self._quit = True
        self.close_file()
        #self._rx_thread.join()

    def _process_packets(self):
        """Use "start" function. Based on a circular buffer."""
        correlator = corr.packet.CasperN_RX_correlator(port=self.port, buffer_size=self.buffer_size,timeout=self.int_time*2)
        pkt = corr.packet.CasperN_Packet(endian='<')

        #total_p_size is the number of words per x engine of one integration
        total_p_size = 2 * self.n_stokes * self.n_bls * self.n_chans/self.n_xeng
        total_integration_words = 2 * self.n_stokes * self.n_bls * self.n_chans
        word_size = pkt.word_size

        timestamps=[0]*self.n_int_buffer
        xeng_data=numpy.zeros([self.n_int_buffer,self.n_xeng,total_p_size])
        xeng_rcvd=numpy.zeros([self.n_int_buffer,self.n_xeng,total_p_size])
        next_write_location = 0
        last_timestamp=0
        total_rcvd=0

        if self.verbose:
            print 'Total integration size: %i words or %i bytes'%(total_integration_words,total_integration_words*word_size)
            print 'Total dump size from each x engine: %i values, %i bytes, %i 128-bit DRAM vectors'%(total_p_size,total_p_size*word_size,total_p_size*word_size/16)

        #get a starting timestamp:
        p = pkt.unpack(correlator.get_a_packet())
        if p == None:
            print 'ERR: couldnt get a good packet to determine starting time'
            exit()
        else:
            timestamp_offset = time.time() - (p['time']*self.timestamp_scale_factor)
            last_timestamp = (int(round(p['time'],self.timestamp_rounding))*self.timestamp_scale_factor) + timestamp_offset

            max_time_allow = self.read_ahead_allow + p['time']
            timestamps = [(p['time']-i) for i in range(self.n_int_buffer)]
            print 'Locked onto timestamp at: ',time.time()

        try:
            while not self._quit:
                #find max permissable mcnt:
                if ((max(timestamps) + self.read_ahead_allow)>max_time_allow):
                    max_time_allow += self.read_ahead_allow
                    if self.verbose:
                        print 'Updated max time allowed to %i'%max_time_allow

                # Get a packet and unpack
                p = pkt.unpack(correlator.get_a_packet())
                if not p == None:

                    if self.verbose:
                        print ' Got offset %i for X engine %i, timestamp %d ' \
                            'with labelled length %i. %i numbers.' %(p['offset'], 
                            p['xeng'],p['time'],p['data_size'], len(p['data']))
                        #check if this is a valid mcnt:
                        if (p['time'] > max_time_allow):
                            print 'Warning: Timestamp of %f very far ahead!'%((p['time']*self.timestamp_scale_factor) + timestamp_offset)


                    #process the packet:
                    #Is this is the first time we've received a packet with this timestamp?
                    #(ie the first packet of new a integration)? 
                    #If so, put it in the next location in the buffer
                    if (timestamps.count(int(round(p['time'],self.timestamp_rounding))) == 0):
                        if self.verbose:
                            print 'Timestamps in buffer: ',timestamps
                            print 'DID NOT FIND THIS TIMESTAMP IN THE BUFFER.' \
                                  ' Got raw %i, rounded to %i. Next write location: %i'\
                                  %(p['time'],int(round(p['time'],self.timestamp_rounding)),next_write_location)
                        timestamps[next_write_location]=int(round(p['time'],self.timestamp_rounding))
                        if next_write_location == (self.n_int_buffer-1):
                            next_write_location = 0
                        else:
                            next_write_location += 1

                    #figure out where to write the packet
                    timestamp_index=timestamps.index(int(round(p['time'],self.timestamp_rounding)))                    

                    start_offset = p['offset']/word_size
                    stop_offset = start_offset + p['data_size']/word_size

                    if self.verbose:
                        print 'Writing to index %i (%i total), %i : %i'%(timestamp_index,total_rcvd,start_offset,stop_offset)
                    xeng_data[timestamp_index][p['xeng']][start_offset:stop_offset]=p['data']
                    xeng_rcvd[timestamp_index][p['xeng']][start_offset:stop_offset]=1
                    total_rcvd = xeng_rcvd[timestamp_index].sum()

                    #Check if we need to save this integration:
                    if total_rcvd==total_integration_words:
                        timestamp = (timestamps[timestamp_index]*self.timestamp_scale_factor) + timestamp_offset
                        print 'Saving integration %i after time %2.2f'%(timestamp_index,timestamp-last_timestamp)
                        last_timestamp = timestamp
                        self.process_integration(xeng_data[timestamp_index],timestamp)
                    #    #reset this data:
                        xeng_data[timestamp_index]=numpy.zeros([self.n_xeng,total_p_size])
                        xeng_rcvd[timestamp_index]=numpy.zeros([self.n_xeng,total_p_size])

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

    config = corr.cn_conf.CorrConf(args[0])
    config_status = config.read_all()
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()

    ANTPOS = [config['antennas']['pos'][a] for a in range(config['n_ants'])]
    ants = [ANTPOS[a] for a in config['antennas']['order']] 
    LOC = config['antennas']['location']
    ADC_CLK = config['adc_clk']
    UDP_PORT = config['rx_udp_port']
    X_PER_FPGA = config['x_per_fpga']
    DDC_MIX_FREQ = config['ddc_mix_freq']
    DDC_DECIMATION = config['ddc_decimation']
    N_CHANS = config['n_chans']
    N_STOKES = config['n_stokes']
    XENG_ACC_LEN = config['xeng_acc_len']
    ACC_LEN = config['acc_len']
    CLK_PER_SYNC = config['clk_per_sync']
    N_ANTS = config['n_ants']
    N_XENG = X_PER_FPGA*len(config['servers'])
    T_PER_FILE = config['t_per_file']
    DB_FILE = config['db_file']
    RX_BUFFER_SIZE = config['rx_buffer_size']

    DEFAULT_EQ=config['eq']['eq_polys']

    n_bls = N_ANTS * (N_ANTS+1.0)/2
    int_time = float(N_CHANS)*XENG_ACC_LEN*ACC_LEN/(ADC_CLK*1000000000/DDC_DECIMATION)

    print 'Processing %.2f MHz bandwidth centred at %.2f MHz'%(ADC_CLK*1000/DDC_DECIMATION,ADC_CLK*1000*DDC_MIX_FREQ)
    print 'Integrating for %.2f seconds'%int_time
    print 'Listening on port %i' %UDP_PORT

    udp_rx = C2M_RX(loc=LOC, n_chans=N_CHANS, ants=ants, n_xeng=N_XENG,
        acc_len=ACC_LEN, adc_clk=ADC_CLK, xeng_acc_len=XENG_ACC_LEN, 
        n_stokes=N_STOKES, ddc_mix_freq=DDC_MIX_FREQ, verbose=opts.verbose,
        ddc_decimation=DDC_DECIMATION, t_per_file=T_PER_FILE,
        dbfile=DB_FILE,eq_polys=DEFAULT_EQ,single_capture=opts.single_capture,
        port=UDP_PORT, buffer_size=RX_BUFFER_SIZE)

    udp_rx.set_bandpass()
    udp_rx.open_file()

    udp_rx.start()

    try:
        while not udp_rx._quit: 
            time.sleep(1)
    except(KeyboardInterrupt):
            udp_rx.stop()
    print '...Exiting.'

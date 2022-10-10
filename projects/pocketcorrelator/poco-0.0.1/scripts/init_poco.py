#! /usr/bin/env python
""" 
Script for initialising a CASPER pocket correlator. Requires poco version 307 or greater. \n
\n
Author: Jason Manley and Sandeep Chaudhari \n
date: 04 Dec 2008\n
\n
2008-12-01  JRM First release  \n
\n
eg usage: ./init_poco.py ~/example.conf \n

"""
import poco, time, sys, numpy, os



def write_ctrl(use_sram_tvg=False, use_fft_tvg=False, arm_rst=False, sync_rst=False, fft_shift=1023):
    """Writes a value to the ibob control register."""
    value = use_sram_tvg<<19 | use_fft_tvg<<18 | arm_rst<<17 | sync_rst<<16 | fft_shift
    brd.write_uint('ctrl',value)

def apply_eq_all(chans, gains):
    """Apply equalization coefficients to the IBOB.
    EQ channels are in the first blocks of address space."""
    brd.write_uint('address',(2**32)-1)
    for ct in range(n_coeff_tables):
        for c, g in zip(chans, gains):
            brd.write_uint('eq_coeff',int(g))
            brd.write_uint('eq_channel',c+(ct*n_chans))
            #print 'Wrote %i for channel %i in coeff table %i.'%(int(g),int(c),int(ct))
    return 'ok'

def apply_eq(coeff_table,chans, gains):
    """Apply equalization coefficients to the IBOB.
    EQ channels are in the first blocks of address space."""
    brd.write_uint('eq_channel',(2**32)-1)
    for c, g in zip(chans, gains):
        brd.write_uint('eq_coeff',int(g))
        brd.write_uint('eq_channel',c+(coeff_table*n_chans))
        #print 'Wrote %i for channel %i in coeff table %i.'%(int(g),int(c),int(ct))
    return 'ok'

def re_equalize(self, thresh=.1, maxval=2**17-1, max_tries=10):
        """Automatically adjust equalization for maximum flatness around 4b pwr of 10. NOT WORKING/UNTESTED"""
        print 'Equalizing'
        for i in range(max_tries):
            d = 0
            for bl in autos: d += read_acc(bl)
            d /= len(autos)
            neweq = numpy.sqrt(numpy.where(d > 0, 10/d, maxval))
            neweq *= equalization
            neweq = numpy.clip(neweq, 0, maxval)
            p = remove_spikes(numpy.log10(neweq+1e-5), return_poly=True)
            neweq = abs(10**numpy.polyval(p, numpy.arange(d.size)))
            neweq = numpy.clip(neweq, 0, maxval)
            fit = math.sqrt(numpy.average((1 - (neweq/equalization))**2))
            print '    Percent gain change:', fit, '(thresh=%f)\n' % thresh
            if fit < thresh: break
            equalization = numpy.round(neweq)
            _apply_eq(active_chans, equalization)
            curr_acc = self['acc_num']
            print '    Waiting for accumulation...'
            while self['acc_num'] <= curr_acc + 1: time.sleep(.01)


if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('init_poco.py [options] CONFIG_FILE')
    p.set_description(__doc__)
    p.add_option('-e', '--auto_eq', dest='init_eq', action='store_true', default=False,
        help='''Attempt to perform automatic equalisation starting from the equalisation polynomials specified in the configuration file. NOT WORKING/UNTESTED''')  
    p.add_option('-i', '--init_eq', dest='init_eq', type='int', default=0,
        help='''Initialise all ibobs' equaliser channels to specified value. Overrides any config file setting.''')
# Added By Sandeep
    p.add_option('-a', '--anta', dest='anta', type='int', default=0,
	help='''Antenna a Delay''')
    p.add_option('-b', '--antb', dest='antb', type='int', default=0,
	help='''Antenna b Delay''')
    p.add_option('-c', '--antc', dest='antc', type='int', default=0,
	help='''Antenna c Delay''')
    p.add_option('-d', '--antd', dest='antd', type='int', default=0,
	help='''Antenna d Delay''')

    opts, args = p.parse_args(sys.argv[1:])

    if args==[]:
        print 'Please specify a configuration file! \nExiting.'
        exit()

    config = poco.poco_conf.CorrConf(args[0])
    config_status = config.read_all()
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()

print'Equalising IBOB on port %s.'%config['connection']['port']

brd = poco.ibob_serial.IbobClient(config['connection']['port'])
n_chans = config['n_chans']
n_coeff_tables = config['n_coeff_tables']
opts.auto_eq = config['auto_eq']
port = config['connection']['port']
acc_len = config['acc_len']
fft_shift = int(config['fft_shift'],16)

if opts.init_eq>0:
    print '''Resetting all ibobs' %i channels' gains to %i...'''%(n_chans,opts.init_eq),
    sys.stdout.flush()
    print apply_eq_all(range(n_chans), [opts.init_eq for i in range(n_chans)])

elif opts.auto_eq:
    print '''Auto equalisation not yet implemented!\n'''
    #print '''Resetting all ibobs' %i channels' gains to %i...'''%(config['n_chans'],opts.auto_eq),
    # Calculate gain in IBOB to extrapolate back to 4b values
    #auto_gain = float(opts.acc_len * (opts.clk_per_sync / n_chan))
    #cross_gain = auto_gain / (2**(4+4*cross_shift))
    # Set a default equalization
    #equalization = numpy.polyval(opts.eq_poly, range(0,opts.n_chan))
    #apply_eq(range(0,opts.n_chan), equalization)

else:
    print 'Configuring static EQ based on config script...'
    for ct in range(n_coeff_tables):
        coeffs = config['equalisation']['eq_polys'][ct]
        print '''Initialising EQ table %i to'''%(ct),
        for term,coeff in enumerate(coeffs):
            if term==(len(coeffs)-1): print '%i...'%(coeff),
            else: print '%ix^%i + '%(coeff,len(coeffs)-term-1),
        sys.stdout.flush()
        equalization = numpy.polyval(coeffs, range(n_chans))
        print apply_eq(ct,range(n_chans), equalization)    

# ARM THE IBOB
print '''ARM the ibob & setting FFT shift...'''
sys.stdout.flush()
write_ctrl(arm_rst=True, fft_shift=fft_shift)

# Sync IBOB without 1PPS signal
print '''Syncing the ibobs & setting FFT shift...'''
sys.stdout.flush()
write_ctrl(sync_rst=True, fft_shift=fft_shift)

# Set the Accumulation Length (in # of syncs)
print ('''Setting the accumulation length to %i...''' %(acc_len))
sys.stdout.flush()
brd.write_uint('acc_len', acc_len)

# Set the Delay for a given Antenna
print ('''Setting Delays to Antennas''')
sys.stdout.flush()
print ('''Setting Delay to Ant A %i...''' %(opts.anta))
brd.write_uint('delay_a',opts.anta)
print ('''Setting Delay to Ant B %i...''' %(opts.antb))
brd.write_uint('delay_b',opts.antb)
print ('''Setting Delay to Ant C %i...''' %(opts.antc))
brd.write_uint('delay_c',opts.antc)
print ('''Setting Delay to Ant D %i...''' %(opts.antd))
brd.write_uint('delay_d',opts.antd)
print 'All done.\n'


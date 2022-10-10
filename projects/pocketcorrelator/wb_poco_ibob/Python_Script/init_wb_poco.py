#! /usr/bin/env python
""" 
Script for initialising a CASPER pocket correlator. Requires poco version wideband_poco_al. \n
\n
Author: Sandeep Chaudhari \n
date: 04 Dec 2008\n
\n
2008-12-01  JRM First release  \n
\n
eg usage: ./init_poco.py ~/example.conf \n

"""
import poco, time, sys, numpy, os


if __name__ == '__main__':
    	from optparse import OptionParser

    	p = OptionParser()
    	p.set_usage('init_poco.py [options] CONFIG_FILE')
    	p.set_description(__doc__)
    	p.add_option('-i', '--init_eq', dest='init_eq', type='int', default=0,
    		help='''Initialise all ibobs' equaliser channels to specified value. Overrides any config file setting.''')

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
port = config['connection']['port']
acc_len = config['acc_len']
fft_shift = int(config['fft_shift'],16)
scale_1 = config['equalisation']['eq_polys'][0]
for term, scale in enumerate(scale_1):		# see help of enumerate
	print 'scale = %i'%scale

# Sync Reset To  IBOB (sync_rst) 
print '''Syncing the ibobs & setting FFT shift...'''
sys.stdout.flush()
brd.write_uint('sync_rst', 1)
brd.write_uint('sync_rst', 0)

#Set Scale of Equaliser (scale & scale)
for term, scale in enumerate(scale_1):
	print 'Scaling The Equaliser of WideBand Pocket Corr......scale_1 = %i, scale_2 = %i'%(scale, scale)
#print '''Scaling The Equaliser of WideBand Pocket Corr...scale_1 = %i, scale_2 = %i'''%(scale_1[0], scale_2[0])
#sys.stdout.flush()
brd.write_uint('scale', scale )
brd.write_uint('scale1', scale )

#Set fft_shift
print ''' Setting FFT Shift ...%i'''%(fft_shift)
sys.stdout.flush()
brd.write_uint('fft_shift', fft_shift)

#Set the Accumulation Length (in # of syncs)
print ('''Setting the accumulation length to %i...''' %(acc_len))
sys.stdout.flush()
brd.write_uint('acc_len', acc_len)

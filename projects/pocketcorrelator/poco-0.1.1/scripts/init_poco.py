#!/usr/bin/python
"""
Initialize CASPER pocket correlator
Author: Griffin Foster 
Date: 2009/09/12
"""

import sys, socket,time
import poco.cn_conf
from optparse import OptionParser

p = OptionParser()
p.set_usage('init_poco.py [options] CONFIG_FILE')
p.set_description(__doc__)

p.add_option('-v', '--verbose', dest='verbose', action='store_true',
    help='Be verbose (good for debugging)')
p.add_option('-e', '--eq', dest='flat_eq', default='',
    help='Set a flat equalization level for all eq blocks and channels')
p.add_option('-o', '--override', dest='override_list', default = '',
    help='Override options in the config file, don\'t use any whitespace, example: -o n_chans=32,n_ants=8')

opts, args = p.parse_args(sys.argv[1:])

if args==[]:
    print 'Please specify a configuration file! \nExiting.'
    exit()

config = poco.cn_conf.CorrConf(args[0], type='poco')
config_status = config.read_all()
print '\nParsing config file %s...%s'%(args[0],config_status)
sys.stdout.flush()
if not config_status == 'OK': exit()

print '\nWriting override variables...',
if not opts.override_list == '':
    oride = opts.override_list.split(',')
    for o in oride:
        oname = o.split('=')[0]
        ovalue = o.split('=')[1]
        if oname in ['fft_shift','rx_ip_address','ibob_ip']: config[oname] = ovalue
        else: config[oname] = int(ovalue)
print 'OK'

fft_shift = config['fft_shift']
n_chans = config['eq_chans']
acc_len = config['acc_len']
rx_ip_address = config['rx_ip_address']
udp_port = config['udp_port']
auto_eq = config['auto_eq']
eq_blocks = config['eq_blocks']
ibob_ip = config['ibob_ip']

HOST = ibob_ip
PORT = 23
print '\nConnecting to iBOB...',
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
print 'OK'
s.send("regwrite ctrl_sw %s\n" % fft_shift)
time.sleep(.1)
s.send("regread ctrl_sw\n")
time.sleep(.1)
s.send("regwrite sync_gen/period %i\n" % acc_len)
time.sleep(.1)
s.send("regread sync_gen/period\n")
time.sleep(.1)
if opts.verbose is True:
    print("regwrite ctrl_sw %s" % fft_shift)
    print("regwrite sync_gen/period %i" % acc_len)

# eq_coeff: bits 0-16
# coeff_en: bit 17
# coeff_addr: bits 20-26
# eq_sel: bits 30-31
for b in range(eq_blocks):
    if not opts.flat_eq == '': eq_level = int(opts.flat_eq)
    else: eq_level = config['eq']['eq_polys'][b][0]
    for c in range(n_chans):
        eq_coeff = b
        
        #shift 10 bits
        eq_coeff *= 2**10
        eq_coeff += c
        
        #shift 3 bits
        eq_coeff *= 2**3
        eq_coeff += 1
        
        #shift 17 bits
        eq_coeff *= 2**17
        eq_coeff += eq_level
        if opts.verbose is True: print 'EQ Block: %i Channel: %i EQ Level: %i' % (b,c,eq_level)
        s.send("regwrite eq_coeff %i\n"% eq_coeff)
        time.sleep(.1)
        s.send("regread eq_coeff\n")
        #print("regwrite eq_coeff %i\n"% eq_coeff)
        time.sleep(.1)
        #print("regread eq_coeff\n")

"""
eq poly
auto eq

[equalisation]
#Starting point for auto-equalization or values for manual eq programming.
#One line entry per antenna. Item is a list of polynomial coefficients.
#Eg,
#EQ_poly_0 = 10, 30 ,40
#corresponds to 10 + 30x^2 + 40x^4
EQ_poly_0 = 1
EQ_poly_1 = 1
EQ_poly_2 = 1
EQ_poly_3 = 1"""

rx_ip_address = rx_ip_address.replace('.',' ')
s.send("startudp %s %i\n" % (rx_ip_address, udp_port))
if opts.verbose is True: print("startudp %s %i\n" % (rx_ip_address, udp_port))
s.close()


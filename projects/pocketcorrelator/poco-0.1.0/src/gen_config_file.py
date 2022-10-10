#!/usr/bin/python
"""
Generate config files for CASPER pocket correlators

Note: this file is dependent on the poco design and how the eq_coeff software register bits
are assigned, modification maybe needed in the amount of shifting in the for loop below

Author: Griffin Foster
Date: 2009/09/09
"""

import sys
from optparse import OptionParser

p = OptionParser()
p.set_usage('gen_config_file.py [options]')
p.set_description(__doc__)

p.add_option('-s', '--shift', dest='fft_shift', default='0x03ff',
    help='FFT shifting in hex, default: 0x03ff')
p.add_option('-p', '--period', dest='period', default=26624000,
    help='Sync gen period for accumulation length, default: 26624000')
p.add_option('-i', '--ip', dest='ip_address', default='169 254 128 10',
    help='Receive computer\'s ip address, default: 169 254 128 10')
p.add_option('-P', '--port', dest='port', default=6969,
    help='Receive computer\'s port, default: 6969')
p.add_option('-e', '--eq', dest='eq_coeff', default=300,
    help='Equalization level coefficents, default: 300')
p.add_option('-n', '--nchan', dest='nchan', default=1024,
    help='Number of channels to write equalization levels for per equalization block, default: 1024')
p.add_option('-b', '--eq_blocks', dest='eq_blocks', default=1,
    help='Number of equalization blocks, default: 1')

opts, args = p.parse_args(sys.argv[1:])

config_file_name = "p%i_c%i_b%i.conf"%(int(opts.eq_coeff), int(opts.nchan), int(opts.eq_blocks))
print "Writing to config file: %s" % config_file_name
file = open(config_file_name, 'w')

file.write("regwrite ctrl_sw %s\n" % opts.fft_shift)
file.write("regread ctrl_sw\n")
file.write("regwrite sync_gen/period %i\n" % int(opts.period))
file.write("regread sync_gen/period\n")


# eq_coeff: bits 0-16
# coeff_en: bit 17
# coeff_addr: bits 20-26
# eq_sel: bits 30-31
for eq_block in range(int(opts.eq_blocks)):
    for c in range(int(opts.nchan)):
        eq_coeff = eq_block
        
        #shift 10 bits
        eq_coeff *= 2**10
        eq_coeff += c
        
        #shift 3 bits
        eq_coeff *= 2**3
        eq_coeff += 1
        
        #shift 17 bits
        eq_coeff *= 2**17
        eq_coeff += int(opts.eq_coeff)
        file.write("regwrite eq_coeff %i\n"% eq_coeff)
        file.write("regread eq_coeff\n")

file.write("startudp %s %i\n" % (opts.ip_address, int(opts.port)));

file.close()
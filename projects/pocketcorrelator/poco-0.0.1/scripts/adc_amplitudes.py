#!/usr/bin/env python

'''
Reads the values of the RMS amplitude accumulators on the ibob through the BEE's XAUI connection.
Requires F engine rev 302 or later and X engine rev 308 or later.

'''
import poco, time, numpy, struct

pol_map=['I','Q']
binary_point=30
adc_num_bits = 8

import poco, time, sys

if __name__ == '__main__':
    from optparse import OptionParser

    p = OptionParser()
    p.set_usage('adc_amplitudes.py [options] CONFIG FILE')
    p.set_description(__doc__)
    opts, args = p.parse_args(sys.argv[1:])


    if args==[]:
        print 'Please specify a configuration file! \nExiting.'
        exit()

    config = poco.poco_conf.CorrConf(args[0])
    config_status = config.read_all()
    print '\n\nParsing config file %s...%s'%(args[0],config_status)
    sys.stdout.flush()
    if not config_status == 'OK': exit()

port = config['connection']['port']

print 'Connecting to IBOB on port %s.'%port

brd = poco.ibob_serial.IbobClient(port)

while(1):
    #get the data...    
    inputs=[{},{},{},{}]
    for input in range(4):
        regname = 'pol%s_sum_sq'%(input+1)
        addr = brd.devs[regname]
        
        #print 'Looking up register: %s at address %i.'%(regname,addr)
        inputs[input]['raw'] = brd._binary_read(size=1,addr=addr)
        inputs[input]['unpacked'] = struct.unpack('>L', inputs[input]['raw'])[0]
        inputs[input]['rms'] = numpy.sqrt(inputs[input]['unpacked']/(2.0**binary_point))
        if inputs[input]['rms'] == 0:
            inputs[input]['bits_used'] = 0        
        else:    
            inputs[input]['bits_used'] = (numpy.log2(inputs[input]['rms'] * (2**(adc_num_bits))))


    #print the output                
    time.sleep(1)
    #Move cursor home:
    print '%c[H'%chr(27)
    #clear the screen:
    print '%c[2J'%chr(27)
    print 'IBOB: ADC0 is bottom (furthest from power port), ADC1 is top (closest to power port).'
    
    print ' ADC amplitudes'
    print '----------------'
    for a in range(2):
        for p in range(2):
            print 'ADC%i input %s: %.3f (%2.2f bits used)'%(a,pol_map[p],inputs[a*2+p]['rms'],inputs[a*2+p]['bits_used'])
           
    print '------------------------------------'

print 'Done with all'
    

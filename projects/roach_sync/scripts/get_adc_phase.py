#!/usr/bin/env python
'''
This script implements measuring the phase difference between the two iADC clocks.
It estimates the phase by sampling a fixed bit pattern with the opposite clock. The opposite
clock is phase-shifted through a range of -180-180 degrees. The clocks are identified as
aligned when an unstable data point is found (multiple values for a single phase), indicating a setup
or hold violation and thus clock phase alignment.

This is based on the scheme used on the iBOBs implemented by Pierre Yves Dros

'''

import corr, time, numpy, struct, sys, logging, stats,socket, pylab

roach='roach0110'
katcp_port=7147
boffile='roach_sync.bof'

def exit_fail():
    print 'FAILURE DETECTED. Log entries:\n',lh.printMessages()
    try:
        fpga.stop()
    except: pass
    raise
    exit()

def exit_clean():
    try:
        for f in fpgas: f.stop()
    except: pass
    exit()

try:
    loggers = []
    lh=corr.log_handlers.DebugLogHandler()
    logger = logging.getLogger(roach)
    logger.addHandler(lh)
    logger.setLevel(10)
    fpga = corr.katcp_wrapper.FpgaClient(roach, katcp_port, timeout=10,logger=logger)

    print('Connecting to server %s on port %i... '%(roach,katcp_port)),
    if fpga.is_connected():
        print 'ok\n'
    else:
        print 'ERROR connecting to server %s on port %i.\n'%(roach,katcp_port)
        exit_fail()
    
    '''
    Bring the phase back 180 degrees
    '''
    for i in range(0,128):
      fpga.blindwrite('iadc_controller','%c%c%c%c'%(0x0,0x03,0x03,0x00))
      time.sleep(0.001) # probably unnecessary wait for delay to take

    '''
     I don't bother to implement the edge weighting (see iBOB code)
    '''
    valA = 0
    valB = 0
    N = 32 # iterations on edge stability detection
    for i in range(0,256):
      toga = 0;
      togb = 0;
      for j in range(0,N):
        val = fpga.read_uint('iadc_controller')
        if (val & 0x01000000):
          toga += 1;
        if (val & 0x02000000):
          togb += 1;
        

      '''
      If we don't have a stable value store the location
      We just use the last occurance
      '''

      if (toga != N and toga != 0) :
        valA = i

      if (togb != N and togb != 0) :
        valB = i

      fpga.blindwrite('iadc_controller','%c%c%c%c'%(0x0,0x01,0x03,0x00))
      time.sleep(0.001) # probably unnecessary wait for delay to take

    '''
    Return to original phase
    '''
    for i in range(0,128):
      fpga.blindwrite('iadc_controller','%c%c%c%c'%(0x0,0x03,0x03,0x00))
      time.sleep(0.001)
    phaseA = valA*360/256
    phaseB = valB*360/256
    '''
    Average the phases and remove offset
    '''
    print 'phase: %d'%((phaseA+phaseB)/2 - 180)

except KeyboardInterrupt:
    exit_clean()
except:
    exit_fail()

exit_clean()


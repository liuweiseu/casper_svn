#!/usr/bin/env python

'''
This code measures the time difference between two pulse per second inputs.
The accuracy is a little dubious and will not give precise sample offsets.
However, it should be good for two samples (180 degrees)
'''
import corr, time, numpy, struct, sys, logging, stats,socket, pylab

roach='roach0110'
katcp_port=7147
boffile='roach_sync.bof'

def highest_set_bit(val) :
  for i in range(0,4) :
    if (val & (1 << 3-i)) :
      return 3-i
  return -99


def skew_calc(first, val0, val1, sync_skew):
  if (first == 2) :
    major_skew = -sync_skew
  else :
    major_skew = sync_skew

  minor_skew = highest_set_bit(val0) - highest_set_bit(val1) 
  
  return 4*major_skew + minor_skew


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

fpga.write_int('rst',0)
fpga.write_int('rst',1)
fpga.write_int('rst',0)
print '---------------------------'
print 'Waiting for sync'
time.sleep(2)
time.sleep(0.01)
sync_source = fpga.read_uint('first_sync_src')
sync0_val = fpga.read_uint('sync0_val')
sync1_val = fpga.read_uint('sync1_val')
sync_skew = fpga.read_uint('sync_skew')
skew = skew_calc(sync_source, sync0_val, sync1_val, sync_skew)
print 'skew = %d'%(skew)

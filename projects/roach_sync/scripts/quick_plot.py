#!/usr/bin/env python

'''
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
    
    print '------------------------'
    print 'Programming FPGA...',
    sys.stdout.flush()
    #fpga.progdev(boffile)
    print 'ok'

    fpga.write_int('snap_adc0_q_ctrl',1)
    fpga.write_int('snap_adc0_q_ctrl',0)
    fpga.write_int('snap_adc1_q_ctrl',1)
    fpga.write_int('snap_adc1_q_ctrl',0)

    fpga.write_int('trig',1)
    fpga.write_int('trig',0)

    snap_adc0_q_int = fpga.read('snap_adc0_q_bram', 64)
    snap_adc0_q = numpy.fromstring(snap_adc0_q_int, count=len(snap_adc0_q_int), dtype='int8')

    snap_adc1_q_int = fpga.read('snap_adc1_q_bram', 64)
    snap_adc1_q = numpy.fromstring(snap_adc1_q_int, count=len(snap_adc1_q_int), dtype='int8')
    pylab.subplot(211)
    pylab.plot(snap_adc0_q[0:10], label='snap_adc0_q')
    pylab.subplot(212)
    pylab.plot(snap_adc1_q[0:10], label='snap_adc1_q')
    pylab.show()
      

except KeyboardInterrupt:
    exit_clean()
except:
    exit_fail()

exit_clean()


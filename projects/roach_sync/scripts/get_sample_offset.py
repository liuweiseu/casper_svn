#!/usr/bin/env python

'''
This code reports the time difference (in samples) between two tones
'''
import corr, time, numpy, struct, sys, logging, stats,socket, pylab

roach='roach0110'
katcp_port=7147
boffile='roach_sync.bof'

#the ADC sample clock frequency
sample_freq = 800e6

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

def tone_freq(data, fs):
    data_len = data.size
    fft_data = numpy.fft.fft(data)
    fft_abs = numpy.abs(fft_data[0:data_len/2]);
    max_pos = numpy.where(fft_abs == fft_abs.max())[0][0]
    return float(max_pos)*float(fs)/float(data_len)

def tone_phase(data):
    data_len = data.size
    fft_data = numpy.fft.fft(data)
    fft_abs = numpy.abs(fft_data[0:data_len/2]);
    max_pos = numpy.where(fft_abs == fft_abs.max())[0][0]
    return numpy.angle(fft_data[max_pos])

def get_tone_phase(fpga):
    accum = 0;
    count = 0;
    
    for i in range(0,2):

      fpga.write_int('snap_adc0_q_ctrl',1)
      fpga.write_int('snap_adc0_q_ctrl',0)
      fpga.write_int('snap_adc1_q_ctrl',1)
      fpga.write_int('snap_adc1_q_ctrl',0)

      fpga.write_int('trig',1)
      fpga.write_int('trig',0)

      snap_adc0_q_int = fpga.read('snap_adc0_q_bram', 4096)
      snap_adc0_q_int = snap_adc0_q_int[0:4096]
      snap_adc0_q = numpy.fromstring(snap_adc0_q_int, count=len(snap_adc0_q_int), dtype='int8')
      phase_adc0_q = tone_phase(snap_adc0_q)

      snap_adc1_q_int = fpga.read('snap_adc1_q_bram', 4096)
      snap_adc1_q_int = snap_adc1_q_int[0:4096]
      snap_adc1_q = numpy.fromstring(snap_adc1_q_int, count=len(snap_adc1_q_int), dtype='int8')
      phase_adc1_q = tone_phase(snap_adc1_q)
      delta = phase_adc0_q - phase_adc1_q

      #unwrap phase
      if (delta > numpy.pi):
        delta = -1*(2*numpy.pi - delta)

      if (delta < -numpy.pi):
        delta = (2*numpy.pi + delta)

      #print 'ph0 %f, ph1 %f, delta %f'%(phase_adc0_q, phase_adc1_q, delta)

      accum = accum + delta
      count = count + 1

    if (count == 0):
      return 3
    return accum/count

def get_tone_freq(fpga, fs):
  fpga.write_int('snap_adc0_q_ctrl',1)
  fpga.write_int('snap_adc0_q_ctrl',0)

  fpga.write_int('trig',1)
  fpga.write_int('trig',0)

  snap_adc0_q_int = fpga.read('snap_adc0_q_bram', 4096)
  snap_adc0_q = numpy.fromstring(snap_adc0_q_int, count=len(snap_adc0_q_int), dtype='int8')
  return tone_freq(snap_adc0_q,fs)

def get_sample_offset(fpga, fs):
  phase=get_tone_phase(fpga)
  freq=get_tone_freq(fpga, fs)

  time_offset = (1/freq) * phase/(2*numpy.pi)
  sample_offset = time_offset/(1/fs)

  #print "avg phase = %f"%(phase)
  #print "tone freq = %f"%(freq)
  #print "time offset = %f ns"%(time_offset*1e9)
  #print "sample offset = %f - < 0 adc1 lags adc0"%(sample_offset)
  if (abs(sample_offset - numpy.round(sample_offset)) > 0.15):
    print 'warning: non integer sample offset'

  return numpy.round(sample_offset)

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
    
    sample_offset = get_sample_offset(fpga,sample_freq)
    print 'sample offset = %d'%(sample_offset)


except KeyboardInterrupt:
    exit_clean()
except:
    exit_fail()

exit_clean()


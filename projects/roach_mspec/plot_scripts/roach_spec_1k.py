#!/usr/bin/python
# this is the server
# ensure sysctl -w net.core.rmem_max=8388608 is set
import socket
import time
import sys
import array
import struct
import numpy as N
import numpy
import Gnuplot

gp = Gnuplot.Gnuplot(persist = 0)
gp('set data style lines')
gp('set term X11')

def get_profile(socket, spectrum_size):
  msg = ''
  msglen = spectrum_size*4; #32 bits per spectrum point
  while len(msg) < msglen:
    chunk = socket.recv(msglen-len(msg))
    if chunk == '':
      raise RuntimeError, "socket connection broken"
    msg = msg + chunk
  return msg

instrument_port   = 10002;
instrument_addr   = ''; #'127.0.0.1'
spectrum_size     = 1024;
profile_averaging = 1;

# bind socket to instrument UDP port
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
s.bind((instrument_addr, instrument_port));

acc_min = 0;
while 1:
  fail=0
  accum=N.zeros(spectrum_size,'I');
  for i in range(0,profile_averaging):
    udp_data = get_profile(s, spectrum_size);
    if (len(udp_data) != spectrum_size*4):
      fail=1;
      print('error: expected %d bytes of data, got %d'%(spectrum_size*4, len(udp_data)));
      break;
    data_unpacked = list(struct.unpack('>%dL'%(spectrum_size), udp_data));
    data = array.array('L');
    data.fromlist(data_unpacked);
    accum += N.array(data, 'I');
  crap = N.max(accum) 
  #foo = (accum == 0)*crap ;
  foo = N.array((accum == 0), 'L');

  poo = N.min(accum + foo*crap);
  if (acc_min == 0) :
    acc_min = poo;
  if (acc_min != 0) :
    if (poo < acc_min) :
      acc_min = poo;
    
  accum = accum + acc_min*foo; 
  #print "min: %d, max: %d"%(poo,crap);
  
  
  plot1=Gnuplot.PlotItems.Data(10*N.log10(accum), inline=0, binary=0)
  #plot1=Gnuplot.PlotItems.Data((accum), inline=0, binary=0)

  gp.plot(plot1);

  if fail :
    break;


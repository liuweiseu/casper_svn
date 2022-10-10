#!/usr/bin/python
# ensure sysctl -w net.core.rmem_max=8388608 is set
import socket
import time
import sys
import array
import struct
import numpy as N
import select
import Gnuplot
import termios
import tty
from sys import stdin


class Spectrum(object):

  def __init__(self):
    self.data = N.zeros((1024,1024),'uint32');
    self.vld  = N.zeros(1024,'uint8');

def safe_recv(socket, size):
  msg = ''
  while len(msg) < (size):
    chunk = socket.recv(size-len(msg))
    if chunk == '':
      raise RuntimeError, "socket connection broken"
    msg = msg + chunk
  return msg

def get_spectrum(socket, spectrum):
  #Set all vlaids to 0
  spectrum.vld = N.zeros(1024,'uint8');
  # Read first 1024 (blocking)
  chunk = safe_recv(socket,4096+8);

  unpacked = struct.unpack('>xxxxI1024I', chunk);
  index = unpacked[0]%1024;
  spectrum.vld[index] = 1;
  data = array.array('L')
  data.fromlist(list((unpacked[1:1025])));
  #spectrum.data[index,:] = data;

  spectrum.data[index,512:1024] = data[0:512];
  spectrum.data[index,0:512] = data[512:1024];
  extras = 0;

  for i in range(1,1024):
    # select with 1/4 second timeout
   result = select.select([socket], [], [], 0.5)
   if (result == ([], [], [])):
      return i;

   chunk = safe_recv(socket,4096+8);
   unpacked = struct.unpack('>xxxxI1024I', chunk);
   index = unpacked[0]%1024;
   spectrum.vld[index] = 1;
   data = array.array('L')
   data.fromlist(list((unpacked[1:1025])));
   #spectrum.data[index,:] = data;
   spectrum.data[index,512:1024] = data[0:512];
   spectrum.data[index,0:512] = data[512:1024];

  while 1:
    # select with 1/4 second timeout
    result = select.select([socket], [], [], 0.1)
    if (result == ([], [], [])):
      break;
    safe_recv(socket,4096+8);
    extras+=1;
    
  return 1024 + extras;

def scale(d, s):
  if s > 0 :
    temp = N.zeros(1024*1024);
    for i in range(0,1024):
      temp[i*1024:(i+1)*1024] = d[i,:];

    return temp;

  ret = N.zeros(1024*4);
  for i in range(0,1024*4):
    moo = d[i/4];
    ret[i] = N.average(moo[(1024/4)*(i%4):(1024/4)*(i%4+1)]);

  return ret;

gp = Gnuplot.Gnuplot(persist = 0)
gp('set data style steps')
gp('set term X11')

#UDP Socket stuff
instrument_port   = 10000;
instrument_addr   = ''; #'127.0.0.1'
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM);
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1048576*8)
s.bind((instrument_addr, instrument_port));

#std in
stdin_fd = sys.stdin.fileno()
old_settings = termios.tcgetattr(stdin_fd)
tty.setcbreak(stdin_fd)


spec_latch = Spectrum();
spec_tot = Spectrum();
spec_int = Spectrum();

averages = 1;
avg_index = 0;
done = 0;
zoom = 0;
pan  = 0;
log  = 0;

acc_str = '';

while done == 0 :
  redraw = 0;
  result = select.select([s, stdin_fd], [], []);
  for fd in result[0]:
    if fd == stdin_fd:
      c = sys.stdin.read(1)
      if c == 'q' :
        done = 1;
      elif c == '+' :
        zoom = min(zoom + 1,1);
      elif c == '-' :
        zoom = max(zoom - 1,0);
      elif c == '*' :
        pan = pan + 1;
      elif c == '/' :
        pan = pan - 1;
      elif c == 'l' :
        log = not log
      elif c == '\n' : # '\n'
        if acc_str != '':
          pan = int(acc_str);
          acc_str = '';
          print '%d %d'%(pan,zoom)
      elif ord(c) >= 48 and ord(c) <= 57:
        acc_str = '%s%c'%(acc_str,c);
      pan = min(max(0,pan), pow(2, zoom*10) - 1)
      redraw = 1;
    if fd == s:
      if get_spectrum(s, spec_int) != 1024 :
        print 'warning: udp over/underflow';
      spec_tot.data += spec_int.data;
      spec_tot.vld += spec_int.vld;

      if (avg_index == averages - 1):
        redraw = 1;
        for i in range(0,1024) :
          if (spec_tot.vld[i]) :
            spec_latch.data[i] = spec_tot.data[i] / spec_tot.vld[i];
            spec_latch.vld[i] = 1;

        spec_tot.data = N.zeros((1024,1024),'uint32');
        spec_tot.vld  = N.zeros(1024,'uint8');
      else:
        avg_index +=1;
        
  if redraw == 1:
    scaled_data = scale(spec_latch.data, zoom);
    if log :
      plot_data = scaled_data[pan*1024:(pan+4)*1024];
      plot_data = 10*N.log(plot_data + (plot_data == 0) * 1e-12);
    else :
      plot_data = scaled_data[pan*1024:(pan+4)*1024];

    plot1=Gnuplot.Data(N.arange(pan*1024*4,pan*1024*4 + 1024*4), plot_data, inline=0, binary=0)
    gp.plot(plot1);

termios.tcsetattr(stdin_fd, termios.TCSADRAIN, old_settings)


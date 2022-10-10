#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from libisigui import *
from libisicorr import *
from libisitvg import *

L=2**7
f1 = sine_wave(L, phase=90)
f2 = sine_wave(L, phase=90)
f3 = sine_wave(L, phase=90)
#n1 = random_noise(L, seed=0xFC96)
#n2 = random_noise(L, seed=0xEB85)
#n3 = random_noise(L, seed=0xDA74)
#s1 = fxn_sum((f1,))
#s2 = fxn_sum((f1,))
#s3 = fxn_sum(([0]*L,))
d1 = interleave_tvg_bram(f1, 16, 2**11)
d2 = interleave_tvg_bram(f2, 16, 2**11)
d3 = interleave_tvg_bram(f3, 16, 2**11)
tv1 = scale_bram_list(d1)
tv2 = scale_bram_list(d2)
tv3 = scale_bram_list(d3)

hosts = ('localhost', 'localhost', 'fake')
ports = (7147, 7148, 7149)
I = IsiCorrelator(hosts, ports)
D = IsiGui(I)

#I.program('isi_correlator_tvg.bof')
I.load_tvg((tv1, tv2, tv3))

I.set_sync_period(2**12)
I.set_fft_shift(0x7f)
I.set_eq_coeff((2**7)<<8)

I.send_sync()

D.start()


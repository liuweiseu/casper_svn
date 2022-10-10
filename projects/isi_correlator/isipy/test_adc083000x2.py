#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

# NOTE: run "ipython --pylab", copy-and-paste, and poke around.
# TODO: automate this.

from libisidebug import *
import time

num_samples = 1<<11

R = IsiCorrelatorDebug('localhost', 7147)
R.progdev('test_adc083000x2.bof')

R.set_clock_freq(200)
R.set_sync_period(1)
R.arm_sync()

R.acquire()

adc = R.read_adc('capt_adc_data', num_samples)


#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from libisidebug import *
from libisitvg import *
import time

def run_test ():

	num_samples = 1<<11

	# Connect to board.
	R = IsiCorrelatorDebug('localhost', 7147)
	R.progdev('test_tvg.bof')

	# Initialize board.
	R.set_clock_freq(200)
	R.set_sync_period(1)
	R.arm_sync()

	# Generate a test vector.
	tvg_sine = sine_wave(num_samples, cycles=1.5)
	tvg_data = scale_bram_data(tvg_sine)
	tvg_bstr = array_to_bytestring(tvg_data)
	R.write('tvg_bram', tvg_bstr)

	# Initiate capture.
	R.acquire()

	# Read data from BRAM.
	data, = R.read_capt('capture', 1, num_samples, signed=True)

	# Verify data.
	status = (list(data) == tvg_data)

	return status

if __name__ == "__main__":
	status = run_test()
	if status:
		print "Passed."
	else:
		print "Failed."


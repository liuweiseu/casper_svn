#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from libisidebug import *
import time

def run_test ():

	num_samples = 1<<11

	# Connect to board.
	R = IsiCorrelatorDebug('localhost', 7147)
	R.progdev('test_capture.bof')

	# Initiate capture.
	R.write_int('control', 0x03)

	# Wait for BRAM to fill.
	done = R.read_int('status')
	while done != 1:
		done = R.read_int('status')
		print "Waiting for BRAM to fill ..."
		time.sleep(.25)

	# Read data from BRAM.
	data, = R.read_capt('capture', 1, num_samples)

	# Verify data.
	status = (list(data) == range(num_samples))

	return status

if __name__ == "__main__":
	status = run_test()
	if status:
		print "Passed."
	else:
		print "Failed."


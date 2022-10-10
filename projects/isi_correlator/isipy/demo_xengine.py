#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from libisidebug import *
from libisitvg import *
import numpy as np

def run_test ():

	num_samples = 1<<11

	# Connect to board.
	R = IsiCorrelatorDebug('localhost', 7147)
	R.progdev('demo_xengine.bof')

	# Initialize board.
	R.set_clock_freq(200)
	R.set_sync_period(1)
	R.arm_sync()
	R.send_sync()

	# Generate a test vector.
	tvg_data = np.zeros(num_samples)
	tvg_data[0] = 1
	# TODO: Make more useful test cases.
	tvg_bstr = array_to_bytestring(tvg_data)
	R.write('tvg_X_bram', tvg_bstr)
	R.write('tvg_Y_bram', tvg_bstr)
	R.write('tvg_Z_bram', tvg_bstr)

	R.reset()

	# Initiate capture.
	R.acquire()

	# Read data from BRAM.
	(XX_r, YY_r, ZZ_r) = R.read_capt("capt_auto_raw", 3, num_samples)
	(XY_r, YZ_r, ZX_r) = R.read_capt("capt_real_raw", 3, num_samples)
	(XY_i, YZ_i, ZX_i) = R.read_capt("capt_imag_raw", 3, num_samples)

	VACC = R._read_bram('vacc_bram', 4096, offset=0)
	print VACC[0:72]

	# Verify data.
	status = (XX_r == YY_r == ZZ_r)

	return status

if __name__ == "__main__":
	status = run_test()
	if status:
		print "Passed."
	else:
		print "Failed."


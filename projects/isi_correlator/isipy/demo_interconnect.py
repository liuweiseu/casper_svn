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

	corr_id = 0

	# Connect to board.
	R = IsiCorrelatorDebug('isi0', 7147)
	R.progdev('demo_interconnect.bof')
	R.write_int('corr_id', corr_id)

	# Initialize board.
	R.set_clock_freq(200)
	R.set_sync_period(1)

	# Generate a test vector.
	R.write('tvg0_bram', tobytestring(counter(num_samples, period=256, offset=0, step=8)))
	R.write('tvg1_bram', tobytestring(counter(num_samples, period=256, offset=1, step=8)))
	R.write('tvg2_bram', tobytestring(counter(num_samples, period=256, offset=2, step=8)))
	R.write('tvg3_bram', tobytestring(counter(num_samples, period=256, offset=3, step=8)))
	R.write('tvg4_bram', tobytestring(counter(num_samples, period=256, offset=4, step=8)))
	R.write('tvg5_bram', tobytestring(counter(num_samples, period=256, offset=5, step=8)))
	R.write('tvg6_bram', tobytestring(counter(num_samples, period=256, offset=6, step=8)))
	R.write('tvg7_bram', tobytestring(counter(num_samples, period=256, offset=7, step=8)))

	# Sync boards.
	R.send_sync()
	time.sleep(1)

	# Initiate capture.
	R.acquire()

	# Read data from BRAM.

	(V,) = R.read_capt("capt_valid", 1, num_samples)
	(S,) = R.read_capt("capt_sync", 1, num_samples)

	# Data sent over XAUI, in its original form.
	(X0, X1, X2) = R.read_capt("capt_012", 3, num_samples)
	(X3, X4, X5) = R.read_capt("capt_345", 3, num_samples)
	(X6, X7, Xz) = R.read_capt("capt_67z", 3, num_samples)

	# Data sent over XAUI, raw.
	(txX, txY, txZ) = R.read_capt8("capt_clump", 3, 4*num_samples)
	# Data sent over XAUI, unscrambled.
	(txX, txY, txZ) = R.unshuffle(corr_id, (txX, txY, txZ))
	(tx0, tx1, tx2) = R.unclump(txX)
	(tx3, tx4, tx5) = R.unclump(txY)
	(tx6, tx7, txz) = R.unclump(txZ)

	# Data received from XAUI, raw.
	(rxX, rxZ, rxY) = R.read_capt8("capt_xaui", 3, 4*num_samples)
	# Data received from XAUI, unscrambled.
	(rx0, rx1, rx2) = R.unclump(rxX)
	(rx3, rx4, rx5) = R.unclump(rxY)
	(rx6, rx7, rxz) = R.unclump(rxZ)
	# NOTE: This data is useless, since it is not sync aligned!

	# Data received from XAUI, resynced and raw.
	(xrX, xrZ, xrY) = R.read_capt8("capt_resync", 3, 4*num_samples)
	# Data received from XAUI, resynced and unscrambled.
	(xrX, xrY, xrZ) = R.unshuffle(corr_id, (xrX, xrY, xrZ))
	(xr0, xr1, xr2) = R.unclump(xrX)
	(xr3, xr4, xr5) = R.unclump(xrY)
	(xr6, xr7, xrz) = R.unclump(xrZ)

	# Data received from XAUI, resynced and unscrambled in hardware.
	(XA, XB, XC) = R.read_capt("capt_X", 3, num_samples)
	(YA, YB, YC) = R.read_capt("capt_Z", 3, num_samples)
	(ZA, ZB, ZC) = R.read_capt("capt_Y", 3, num_samples)

	# Verify data.

	# These should hold:
	# X0 = tx0 = xr0 = XA

	correct_stf = \
		(X0 == XA) & \
		(X1 == XB) & \
		(X2 == XC) & \
		(X3 == YA) & \
		(X4 == YB) & \
		(X5 == YC) & \
		(X6 == ZA) & \
		(X7 == ZB) & \
		(Xz == ZC)
	if not correct_stf:
		print "Failure somewhere."

	correct_tx = \
		(X0 == tx0) & \
		(X1 == tx1) & \
		(X2 == tx2) & \
		(X3 == tx3) & \
		(X4 == tx4) & \
		(X5 == tx5) & \
		(X6 == tx6) & \
		(X7 == tx7) & \
		(Xz == txz)
	if not correct_tx:
		print "Failure in clump block."

	correct_xr = \
		(tx0 == xr0) & \
		(tx1 == xr1) & \
		(tx2 == xr2) & \
		(tx3 == xr3) & \
		(tx4 == xr4) & \
		(tx5 == xr5) & \
		(tx6 == xr6) & \
		(tx7 == xr7) & \
		(txz == xrz)
	if not correct_xr:
		print "Failure in XAUI or resync block."

	correct_X = \
		(xr0 == XA) & \
		(xr1 == XB) & \
		(xr2 == XC) & \
		(xr3 == YA) & \
		(xr4 == YB) & \
		(xr5 == YC) & \
		(xr6 == ZA) & \
		(xr7 == ZB) & \
		(xrz == ZC)
	if not correct_X:
		print "Failure in unclump block."

	status = (correct_tx & correct_xr & correct_X)

	return status

if __name__ == "__main__":
	status = run_test()
	if status:
		print "Passed."
	else:
		print "Failed."


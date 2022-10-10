#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from cmd import Cmd
from isi_roach_ctrl import IsiRoachCtrl
from libisifengine import IsiFEngine

class IsiFEngineCtrl (IsiRoachCtrl, Cmd):

	prompt = "isi> "

	def __init__ (self, addr, port=7147):
		Cmd.__init__(self)

		self._board = IsiFEngine(addr, port)
		if port == 7147:
			print "Connected to %s." % (addr)
		else:
			print "Connected to %s:%d." % (addr, port)
		self._board.initialize()

	#
	# FEngine Commands
	#

	def do_sync_period (self, line):
		args = line.split()
		if len(args) == 0:
			print "Sync Period: %fs" % self._board.sync_period
			return
		if len(args) != 1:
			print "Too many arguments."
			return
		s_val = args.pop(0)
		try:
			val = float(s_val)
		except ValueError:
			print "Invalid value: %s" % s_val
			return
		self._board.set_sync_period(val)

	def do_fft_shift (self, line):
		args = line.split()
		if len(args) == 0:
			print "FFT Shift: 0x%02x" % self._board.fft_shift
			return
		if len(args) != 1:
			print "Too many arguments."
			return
		s_val = args.pop(0)
		try:
			val = int(s_val, 0)
		except ValueError:
			print "Invalid value: %s" % s_val
			return
		self._board.set_fft_shift(val)

	def do_eq_coeff (self, line):
		args = line.split()
		if len(args) == 0:
			print "Eq Coeff: %d" % self._board.eq_coeff
			return
		if len(args) != 1:
			print "Too many arguments."
			return
		s_val = args.pop(0)
		try:
			val = int(s_val, 0)
		except ValueError:
			print "Invalid value: %s" % s_val
			return
		self._board.set_eq_coeff(val)

	def do_reinit (self, line):
		self._board.initialize()

	#
	# Debug Commands
	#

	def do_fill_tvgs (self, line):
		"""fill_tvgs [tvg_name] [num_tvgs] [tvg_size] [length]"""

		args = line.split()

		if len(args) == 0:
			print "Need a tvg name."
			return
		tvg_name = args.pop(0)

		if len(args) == 0:
			print "Need the number of tvgs."
			return
		s_num_tvgs = args.pop(0)
		try:
			num_tvgs = int(s_num_tvgs)
		except ValueError:
			print "Invalid value: %s" % s_num_tvgs
			return

		if len(args) == 0:
			print "Need the size of tvgs."
			return
		s_tvg_size = args.pop(0)
		try:
			tvg_size = int(s_tvg_size)
		except ValueError:
			print "Invalid value: %s" % s_tvg_size
			return

		if len(args) == 0:
			print "Need a counter length."
			return
		s_length = args.pop(0)
		try:
			length = int(s_length)
		except ValueError:
			print "Invalid value: %s" % s_length
			return

		import libisitvg
		A = libisitvg.np.arange(length)
		B = libisitvg.interleave_tvgs (A, num_tvgs, tvg_size)
		C = libisitvg.np.array(B, dtype='>i4')

		for i in xrange(num_tvgs):
			bram_name = "%s%d_bram" % (tvg_name, i)
			self._board.write(bram_name, C[i].tostring())

	def do_histogram (self, line):
		args = line.split()
		if len(args) == 0:
			print "Specify the desired number of samples."
			return
		if len(args) != 1:
			print "Too many arguments."
			return
		s_val = args.pop(0)
		try:
			val = int(s_val, 0)
		except ValueError:
			print "Invalid value: %s" % s_val
			return

		print "Grabbing %d samples." % val
		import numpy as np
		histR = np.zeros(val, dtype=np.int32)
		histI = np.zeros(val, dtype=np.int32)
		for i in xrange(val):
			hist_raw = self._board.read_int('equalizer_sample')
			histR[i] = self._sign_extend(hist_raw>>4 & 0x0f, 4)
			histI[i] = self._sign_extend(hist_raw>>0 & 0x0f, 4)

		print "Dumping to file."
		self._dump_to_file("histR", histR)
		self._dump_to_file("histI", histI)

	#
	# Misc helper methods.
	#

	def _sign_extend (self, data, bits):
		return (data + 2**(bits-1)) % 2**bits - 2**(bits-1)

	def _dump_to_file (self, name, data):
		from time import strftime
		timestamp = strftime("%Y%m%dT%H%M%S")
		filename = "data/%s_%s.dump" % (timestamp, name)

		f = open(filename, "w")
		data.tofile(f, sep="\n", format="%d")
		f.close()

def main ():
	import sys

	sys.argv.pop(0)

	if len(sys.argv) == 0:
		print "Must specify a hostname."
		return
	host = sys.argv.pop(0)

	port = 7147
	if len(sys.argv) != 0:
		s_port = sys.argv.pop(0)
		try:
			port = int(s_port)
		except ValueError:
			print "Invalid port number: %s" % s_port
			return

	IsiFEngineCtrl(host, port).cmdloop()

if __name__ == '__main__':
	main()


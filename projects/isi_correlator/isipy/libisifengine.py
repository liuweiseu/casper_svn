#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from libisiroach import IsiRoach

class IsiFEngine (IsiRoach):

	def __init__ (self, host, port=7147, id=-1):
		IsiRoach.__init__(self, host, port)

		self._id = id
		self._clock_freq = 0

	def get_clock_freq (self):
		return self._clock_freq

	def get_sync_period (self):
		clks = self.read_int('sync_gen_period')
		sync_period = float(clks) / (self._clock_freq * 10**6)
		return sync_period

	def set_sync_period (self, period):
		clks = int(period * self._clock_freq * 10**6)
		self.write_int('sync_gen_period', clks)

	def get_fft_shift (self):
		fft_shift = self.read_int('fft_shift')
		return fft_shift

	def set_fft_shift (self, shift):
		self.write_int('fft_shift', shift)

	def get_eq_coeff (self):
		eq_coeff = self.read_int('eq_coeff')
		return eq_coeff

	def set_eq_coeff (self, coeff):
		self.write_int('eq_coeff', coeff)

	def initialize (self):
		board_is_programmed = True
		try:
			self.status()
		except RuntimeError:
			board_is_programmed = False

		if board_is_programmed:
			print "Status: Programmed."

			self._clock_freq = self.est_brd_clk()
			self._clock_freq -= self._clock_freq % 5

			if self._clock_freq > 0:
				print "* Clock freq = %d MHz" % self._clock_freq
			else:
				print "No clock is connected to this board."

			try:
				sync_period = self.get_sync_period()
				print "* Sync Period = %fs" % sync_period

				fft_shift = self.get_fft_shift()
				print "* FFT Shift = 0x%x" % fft_shift

				eq_coeff = self.get_eq_coeff()
				print "* EQ Coeff = %d" % eq_coeff
			except RuntimeError:
				print "WARN: Failed to read some registers."
		else:
			print "Status: Unprogrammed."


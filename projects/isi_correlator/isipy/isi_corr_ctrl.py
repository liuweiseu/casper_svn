#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from cmd import Cmd
from any_roach_ctrl import AnyRoachCtrl
from libisifengine import IsiFEngine

class IsiCorrCtrl (AnyRoachCtrl):

	prompt = "isi_corr> "

	def __init__ (self):
		AnyRoachCtrl.__init__(self, ctor=IsiFEngine)

	#
	# FEngine Commands
	#

	def do_enumerate (self, line):
		"""enumerate
		Set the corr_id for each board sequentially.
		"""
		for id in xrange(len(self._boards)):
			self._boards[id].write_int('corr_id', id)
		print "Boards enumerated."

	def do_arm (self, line):
		"""arm
		Arm all currently loaded boards.
		"""
		for id in self._selected_ids:
			self._boards[id].write_int('control', 4)
		for id in self._selected_ids:
			self._boards[id].write_int('control', 1)
		for id in self._selected_ids:
			self._boards[id].write_int('control', 0)
		print "Boards armed."

	def do_sync_period (self, line):
		args = line.split()

		# Check for an argument.
		try:
			s_val = args.pop(0)
		except IndexError:
			pass
		else:
			# If there is an arg,
			# set the sync period.
			try:
				val = float(s_val)
			except ValueError:
				print "Invalid value: %s" % s_val
			else:
				for id in self._selected_ids:
					self._boards[id].set_sync_period(val)

		# Get the sync period.
		for id in self._selected_ids:
			period = self._boards[id].get_sync_period()
			print "[%d] Sync Period: %fs" % (id, period)

	def do_fft_shift (self, line):
		args = line.split()

		# Check for an argument.
		try:
			s_val = args.pop(0)
		except IndexError:
			pass
		else:
			# If there is an arg,
			# set the fft shift.
			try:
				val = int(s_val, 0)
			except ValueError:
				print "Invalid value: %s" % s_val
			else:
				for id in self._selected_ids:
					self._boards[id].set_fft_shift(val)

		# Get the fft shift.
		for id in self._selected_ids:
			fft_shift = self._boards[id].get_fft_shift()
			print "[%d] FFT Shift: 0x%02x" % (id, fft_shift)

	def do_eq_coeff (self, line):
		args = line.split()

		# Check for an argument.
		try:
			s_val = args.pop(0)
		except IndexError:
			pass
		else:
			# If there is an arg,
			# set the eq coeff.
			try:
				val = int(s_val, 0)
			except ValueError:
				print "Invalid value: %s" % s_val
			else:
				for id in self._selected_ids:
					self._boards[id].set_eq_coeff(val)

		# Get the eq coeff.
		for id in self._selected_ids:
			eq_coeff = self._boards[id].get_eq_coeff()
			print "[%d] Eq Coeff: %d" % (id, eq_coeff)


	def do_reinit (self, line):
		for id in self._selected_ids:
			self._boards[id].initialize()

	#
	# Debug Commands
	#

	def do_histogram (self, line):
		args = line.split()

		try:
			arg1 = args.pop(0)
			arg2 = args.pop(0)
		except IndexError:
			print "Usage: histogram [sample_type] [num_samples]"
			print "  where sample_type = adc|pfb|eq"
			return

		sample_type = arg1 + '_sample'
		s_num_samples = arg2
		try:
			num_samples = int(s_num_samples, 0)
		except ValueError:
			print "Invalid value: %s" % s_num_samples
			return

		print "Grabbing %d %s samples." % (num_samples, sample_type)
		import numpy as np
		histR = np.zeros(num_samples, dtype=np.int32)
		histI = np.zeros(num_samples, dtype=np.int32)
		for i in xrange(num_samples):
			# TODO: add multiboard support.
			hist_raw = self._boards[0].read_int('equalizer_sample')
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

if __name__ == '__main__':
	IsiCorrCtrl().cmdloop()


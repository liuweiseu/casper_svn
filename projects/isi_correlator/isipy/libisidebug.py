#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import libisiroach

import corr
import itertools
import pylab
import time
import struct
import sys

from libisidatarcvr import *

class IsiCorrelatorDebug (libisiroach.IsiRoachBoard):
	def __init__ (self, host, port):
		super(IsiCorrelatorDebug, self).__init__(host, port)

		self._data_rcvr = []
		self._data_l = []
		self._figure = None
		time.sleep(.1)

	def gen_plot (self, data_l):
		assert (type(data_l) == list)

		if (type(data_l[0]) != list):
			self._data_l = [data_l]
		else:
			self._data_l = data_l

		num_plots = len(self._data_l)
		plot_delta = 1. / num_plots
		plot_offset = 1.0
		plot_rect = [0.04, plot_offset, 0.96, plot_delta]

		axprops = dict(xticklabels=[], yticklabels=[])
		self._figure = pylab.figure()

		c_list = []
		for i in xrange(num_plots):
			data = self._data_l[i]
			plot_rect[1] -= plot_delta
			ax = self._figure.add_axes(plot_rect, **axprops)
			c, = ax.plot(data, '.')
			ax.autoscale_view(tight=True, scalex=True, scaley=True)

		return c_list

	#
	# Block verification methods.
	#

	def bram_interleave (self, data):
		"""Interleaves parallel BRAM data streams."""
		iter = []
		for i in data:
			iter += [itertools.chain(i)]
		return itertools.izip(*iter)

	def flatten (self, iterlist):
		"""Flattens an iterator of iterators into just an iterator."""
		return itertools.chain(*iterlist)

	def unclump (self, A):
		"""Inverts the compression operation of the clump block."""
		X = []
		num_frames = len(A)/16
		for i in xrange(num_frames):
			sof = i*16 + 4
			eof = sof + 12
			X += A[sof:eof]
		X1 = X[0:None:3]
		X2 = X[1:None:3]
		X3 = X[2:None:3]
		return (tuple(X1), tuple(X2), tuple(X3))

	def unshuffle (self, id, A):
		"""Inverts the reordering operation of the shuffle block."""
		if id == 0:
			return (A[0], A[1], A[2])
		if id == 1:
			return (A[2], A[0], A[1])
		if id == 2:
			return (A[1], A[2], A[0])
		return None

	def sign_extend (self, data, bits):
		return (data + 2**(bits-1)) % 2**bits - 2**(bits-1)

	#
	# Block readout functions
	#

	def read_adc (self, capt_block, read_len):
		length = read_len / 16
		bram_data = self.read_capt(capt_block, 16, length, signed=True)
		data_iter = self.bram_interleave(bram_data)
		data = self.flatten(data_iter)
		return [self.sign_extend(x, 8) for x in data]

	def read_pfb (self, capt_block, read_len):
		length = read_len / 16
		bram_data = self.read_capt(capt_block, 16, length, signed=True)
		data_iter = self.bram_interleave(bram_data)
		data = self.flatten(data_iter)
		return [self.sign_extend(x, 18) for x in data]

	def read_fft (self, capt_block, read_len):
		length = read_len / 8
		bram_data = self.read_capt(capt_block, 8, length)
		data_iter = self.bram_interleave(bram_data)
		data = self.flatten(data_iter)
		return [x for x in data]


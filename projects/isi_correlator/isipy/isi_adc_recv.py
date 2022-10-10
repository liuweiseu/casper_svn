#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np
import sys

from isi_data_recv import IsiDataRecv

num_boards = 2
num_samples = 256

pktfmt = np.dtype \
([  
	('board_id', '>i1'),
	('unused1', '>i1'),
	('unused2', '>i1'),
	('unused3', '>i1'),
	('pkt_id', '>i4'),
	('adc0', '>i4', num_samples/2),
	('adc1', '>i4', num_samples/2),
])

datafmt = np.dtype \
([
	('pkt_id', '>i4'),
	('adc', '>i4', (num_boards, num_samples)),
])

class IsiAdcRecv (IsiDataRecv):

	def __init__ (self):
		IsiDataRecv.__init__(self, pktfmt, datafmt)

		self._buf_length = 4
		self._max_pktid = 0
		self._lim_pktid = 1<<32 # for a 32 bit pktid (from FPGA code)

		len_pktid = self._buf_length * num_boards
		dim_pktid = (self._buf_length, num_boards)
		self._PKTID = np.arange(len_pktid, dtype='>i4').reshape(dim_pktid)

		len_data = self._buf_length * num_boards * num_samples
		dim_data = (self._buf_length, num_boards, 2, num_samples/2)
		self._BUFFER = np.zeros(len_data, dtype='>i4').reshape(dim_data)

	def descramble (self):
		board = self._PKT['board_id'][0]
		pktid = self._PKT['pkt_id'][0]

		slot = pktid % self._buf_length

		if board >= num_boards:
			print "Ignoring data from board %d." % board
			return False

		self._PKTID[slot][board] = self._PKT['pkt_id'].copy()
		self._BUFFER[slot][board][0] = self._PKT['adc0'].copy()
		self._BUFFER[slot][board][1] = self._PKT['adc1'].copy()

		# find full sets and process them.
		packet_ready = False
		for i in xrange(self._buf_length):
			s = (self._max_pktid + i) % self._buf_length
			if self._PKTID[s].ptp() == 0:
				self.process_pkt(self._PKTID[s,0], self._BUFFER[s])
				self._max_pktid = (self._max_pktid + i) % self._lim_pktid
				# Taint the row so that it doesn't get
				# read again on a packet counter reset.
				self._PKTID[s,0] += 1
				packet_ready = True
				break

		return packet_ready

	def process_pkt (self, pktid, datum):
		x = datum.reshape(num_boards,16,-1)
		y = x.transpose(0,2,1)
		z = y.reshape(num_boards,num_samples)

		#import IPython
		#ipshell = IPython.Shell.IPShellEmbed()
		#ipshell()

		self._DATA['adc'] = z

		if self._dumpfile:
			self._DATA.tofile(self._dumpfile)
			self._dumpfile.close()
			self._dumpfile = None
			print "Dumpfile closed."

if __name__ == "__main__":
	IsiAdcRecv().main()


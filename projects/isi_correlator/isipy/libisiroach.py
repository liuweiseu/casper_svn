#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from corr import katcp_wrapper as katcp
from time import sleep

class IsiRoach (katcp.FpgaClient):
	"""
	An ISI-specific extension of the KATCP FPGA Client class.
	"""

	ARM     = 1<<0
	TRIG    = 1<<1
	RESET   = 1<<2

	def __init__ (self, host, port=7147):
		katcp.FpgaClient.__init__(self, host, port)
		sleep(.25)

		self._host = host
		self._port = port

	def zero_ctrl (self):
		self.write_int('control', 0)

	def arm (self):
		self.write_int('control', IsiRoach.ARM)

	def trig (self):
		self.write_int('control', IsiRoach.TRIG)

	def reset (self):
		self.write_int('control', IsiRoach.RESET)

	def get_control (self):
		return self.read_int('control')

	def get_status (self):
		return self.read_int('status')


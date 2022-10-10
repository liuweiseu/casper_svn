#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import socket
import sys

import numpy as np
from time import strftime

DATA_SOCK = "/tmp/isi_data_sock"
CTRL_SOCK = "/tmp/isi_ctrl_sock"

class IsiDataView ():
	"""Abstract class to view packets from an IsiDataRecv instance.

	Parameters
	----------
	datafmt : np.dtype
		A numpy struct to hold descrambled ROACH data.

	Abstract Methods
	----------------
	view_data() :
		* Takes no arguments.
		* Somehow displays _DATA.
		* Returns nothing.
	"""

	def __init__ (self, datafmt):
		self._DATA = np.zeros(1, dtype=datafmt)

	def view_data (self):
		raise NotImplementedError

	def main (self):

		data_sock = self._open_data_client_sock(DATA_SOCK)
		ctrl_sock = self._open_ctrl_client_sock(CTRL_SOCK)
		ctrl_sock.send("subscribe %s" % DATA_SOCK)
		data_sock.recv_into(self._DATA)
		self._close_data_client_sock(data_sock)
		self._close_ctrl_client_sock(ctrl_sock)

		self.view_data()

	def _open_data_client_sock (self, sockname):
		sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
		try:
			sock.bind(sockname)
		except socket.error:
			print "WARN: %s exists! Removing." % sockname
			socket.os.unlink(sockname)
			sock.bind(sockname)
		print "Opened data socket."
		return sock

	def _open_ctrl_client_sock (self, sockname):
		sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
		try:
			sock.connect(sockname)
		except socket.error:
			print "ERROR: Cannot connect to recv server."
			sys.exit(1)
		print "Opened ctrl socket."
		return sock

	def _close_data_client_sock (self, sock):
		sockname = sock.getsockname()
		sock.close()
		socket.os.unlink(sockname)
		print "Closed data socket."

	def _close_ctrl_client_sock (self, sock):
		sock.close()
		print "Closed ctrl socket."


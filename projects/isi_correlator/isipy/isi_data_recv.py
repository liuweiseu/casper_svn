#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

import select
import socket
import struct
import sys

import numpy as np

import IPython
ipshell = IPython.Shell.IPShellEmbed()

CTRL_SOCK = "/tmp/isi_ctrl_sock"

class IsiDataRecv ():
	"""Abstract class to receive packets from an ISI roach board.

	Parameters
	----------
	addr : string
		The address of the receiving network interface.
	port : int
		The port to listen on for data from the ROACH.
	pktfmt : np.dtype
		A numpy struct to hold raw data from the ROACH.
	datafmt : np.dtype
		A numpy struct to hold descrambled ROACH data.

	Abstract Methods
	----------------
	descramble() :
		* Takes no arguments.
		* Transforms _PKT to _DATA.
		* Returns a bool: True iff a full dataset is now ready.
	"""

	def __init__ (self, pktfmt, datafmt):

		(addr, port) = self._parse_opts()

		self._addr = addr
		self._port = port
		self._recv_sock = None
		self._ctrl_sock = None

		self._PKT = np.zeros(1, dtype=pktfmt)
		self._DATA = np.zeros(1, dtype=datafmt)

		self._ilist = None
		self._olist = None

		self.not_killed = True

		self._dumpfile = None

		self._plotdata = None

	def descramble (self):
		raise NotImplementedError

	def main (self):
		"""The main loop. Receives and processes data and commands.

		* Receives data from remote ROACH boards via a UDP socket.
		* Receives commands from local processes via a UNIX socket,
		  and sends data to local processes as requested.
		* Receives user input from the console.
		"""

		self._open_recv_socket(self._addr, self._port)
		self._open_ctrl_socket(CTRL_SOCK)

		self._ilist = [self._recv_sock, self._ctrl_sock, sys.stdin]
		self._olist = []

		packet_ready = False

		print "Entering receive loop."

		while self.not_killed:
			irdy, ordy, erdy = select.select(self._ilist, self._olist, [])

			for iobj in irdy:

				if iobj == self._recv_sock:
					self._recv_sock.recv_into(self._PKT)
					packet_ready = self.descramble()
					#print "Got packet #%d." % self._PKT['pkt_id'][0]

				elif iobj == self._ctrl_sock:
					print "A new control stream has connected."
					client, address = self._ctrl_sock.accept()
					self._ilist.append(client)

				elif iobj == sys.stdin:
					raw_cmd = ""
					raw_cmd = iobj.readline()
					if len(raw_cmd) == 0:
						print "Caught Ctrl-D."
						self.not_killed = False
					else:
						args = raw_cmd.split()
						if args[0] == "dump":
							try:
								filename = args[1]
							except IndexError:
								print "Must specify a filename."
							else:
								self._dumpfile = open(filename, "w")
								print "Dumpfile opened."
						elif args[0] == "stop":
							self._dumpfile.close()
							self._dumpfile = None
							print "Dumpfile closed."
						elif args[0] == "plot":
							try:
								plotname = args[1]
							except IndexError:
								print "Must specify a field name."
							else:
								try:
									self._plotdata = self._DATA[plotname][0]
								except ValueError:
									print "Field '%s' does not exist." % plotname
								else:
									plt.ion()
									print "Plotting %s." % plotname
						elif args[0] == "plotoff":
							self._plotdata = None
							plt.close()
							plt.ioff()
							print "Not plotting."
						else:
							print "Unrecognized command: %s" % args[0]

				else:
					data = iobj.recv(1024)
					if data:
						self._handle_command(iobj, data)
					else:
						print "A control stream has disconnected."
						iobj.close()
						self._ilist.remove(iobj)

			if packet_ready:
				packet_ready = False
				for oobj in ordy:
					try:
						oobj.send(self._DATA)
					except socket.error:
						print "A data stream has disconnected."
						oobj.close()
						self._olist.remove(oobj)
				if self._plotdata != None:
					plt.clf()
					for data in self._plotdata:
						plt.plot(data)
					plt.draw()

		print "Exited receive loop."

		self._close_ctrl_socket()
		self._close_recv_socket()

	def _parse_opts (self):

		try:
			raw_host = sys.argv[1]
			raw_port = sys.argv[2]
		except IndexError:
			print "Usage: %s [host] [port]" % sys.argv[0]
			sys.exit(1)

		try:
			host = str(raw_host)
			port = int(raw_port)
		except ValueError:
			print "Invalid argument."
			sys.exit(1)

		return (host, port)

	def _open_recv_socket (self, addr, port):
		self._recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self._recv_sock.bind((addr, port))
		print "Opened recv socket."

	def _open_ctrl_socket (self, sockname):
		self._ctrl_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
		try:
			self._ctrl_sock.bind(sockname)
		except socket.error, e:
			print "WARN: ctrl_sock exists! Removing."
			socket.os.unlink(sockname)
			self._ctrl_sock.bind(sockname)
		self._ctrl_sock.listen(5)
		print "Opened ctrl socket."

	def _open_send_socket (self, name):
		sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
		try:
			sock.connect(name)
		except socket.error, e:
			print "ERROR: Cannot connect to %s." % name
			sock.close()
			sock = None
		return sock
		print "Opened send socket."

	def _close_recv_socket (self):
		self._recv_sock.close()
		print "Closed recv socket."

	def _close_ctrl_socket (self):
		sockname = self._ctrl_sock.getsockname()
		self._ctrl_sock.close()
		socket.os.unlink(sockname)
		print "Closed ctrl socket."

	def _handle_command (self, iobj, data):
		args = data.lstrip().strip().split()
		cmd = args.pop(0).lower()

		if cmd == "subscribe":
			print "Subscribing: %s" % args
			name = args.pop(0)
			sock = self._open_send_socket(name)
			if sock:
				print "A data stream has connected."
				self._olist.append(sock)

		elif cmd == "dump":
			print "Dumping to: %s" % args
			iobj.send(self._DATA)

		else:
			print "Unknown command: %s" % data


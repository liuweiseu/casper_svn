#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import pygtk
pygtk.require('2.0')
import gtk
import gobject

import libisiplot
import libisiroach
import socket
import sys

import numpy as np

from time import strftime

class IsiFEngineGui (gtk.Window):
	"""The control and status GUI for the ISI F-Engine."""

	def __init__ (self, board, sock):
		gtk.Window.__init__(self)

		self._board = board
		self._sock = sock

		self._canvas = None
		self._figure = None

		self._set_params()
		self._set_accels()
		self._set_layout()

		self._datafmt = np.dtype \
	    ([
	        ('pkt_id', '>i4'),
	        ('adc', '>i4', 512),
	        ('pfb', '>i4', 128),
	        ('fftR', '>i4', 64),
	        ('fftI', '>i4', 64),
	        ('raR', '>i4', 64),
	        ('raI', '>i4', 64),
	    ])

		self._DATA = np.zeros(1, dtype=self._datafmt)

		self._dump_pending = False

	def _set_params (self):
		self.set_default_size(800, 600)
		self.connect("destroy", self._quit_action)
		self.set_title("ISI F-Engine")
		self.set_border_width(8)

	def _set_accels (self):
		group = gtk.AccelGroup()

		cmd_map = dict \
		({ \
			'<Control>q' : self._quit_action, \
			'<Control>f' : self._fullscreen_action, \
			'<Control>d' : self._filedump_action, \
		})

		for (cmd, fxn) in cmd_map.items():
			key, mod = gtk.accelerator_parse(cmd)
			group.connect_group(key, mod, gtk.ACCEL_VISIBLE, fxn)

		self.add_accel_group(group)

	def _set_layout (self):
		self._figure = libisiplot.IsiFigure()
		self._canvas = libisiplot.IsiCanvas2(self._figure, 3, 2)
		self.add(self._canvas)

		self._canvas.add_plot(0, [0]*512, "adc", line='-')
		self._canvas.add_plot(1, [0]*128, "pfb", line='-')
		self._canvas.add_plot(2, [0]*64, "fft_r")
		self._canvas.add_plot(3, [0]*64, "fft_i")
		self._canvas.add_plot(4, [0]*64, "ra_r")
		self._canvas.add_plot(5, [0]*64, "ra_i")

	def _quit_action (self, widget, *args):
		gtk.main_quit()
		return True

	def _fullscreen_action (self, widget, *args):
		self.maximize()
		return True

	def _filedump_action (self, widget, *args):
		self._dump_pending = True
		return True

	def _dump_to_file (self, data, name, timestamp):
		filename = "%s_%s.dump" % (timestamp, name)

		f = open(filename, "w")
		for i in data:
			f.write('%d\n' % i)
		f.close()

	def _update (self):
		self._sock.recv_into(self._DATA)

#		if self._dump_pending:
#			print "Dumping plot data to file!"
#			timestamp = strftime("%Y%m%dT%H%M%S")
#
#			self._dump_to_file(adc, "adc", timestamp)
#			self._dump_to_file(pfb, "pfb", timestamp)
#			self._dump_to_file(fft, "fft", timestamp)
#			self._dump_to_file(eq, "eq", timestamp)
#
#			self._dump_pending = False

		self._canvas.set_data(0, self._DATA['adc'][0])
		self._canvas.set_data(1, self._DATA['pfb'][0])
		self._canvas.set_data(2, self._DATA['fftR'][0])
		self._canvas.set_data(3, self._DATA['fftI'][0])
		self._canvas.set_data(4, self._DATA['raR'][0])
		self._canvas.set_data(5, self._DATA['raI'][0])

		self._canvas.draw()
		return True

	def start (self):
		gobject.idle_add(self._update)
		self.show_all()
		gtk.main()

if __name__ == "__main__":
	R = libisiroach.IsiRoachBoard('localhost', 7147)
#	print "Programming board."
#	R.progdev('demo_fengine.bof')
	print "Configuring board."
	R.initialize()
	R.set_sync_period(.25)
	R.set_fft_shift(0x7f)
	R.set_eq_coeff(1<<11)
	R.arm_sync()

	DATA_SOCK = "/tmp/isi_push_sock"
	CTRL_SOCK = "/tmp/isi_ctrl_sock"

	data_sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
	try:
		data_sock.bind(DATA_SOCK)
	except socket.error:
		print "WARN: %s exists! Removing." % DATA_SOCK
		socket.os.unlink(DATA_SOCK)
		data_sock.bind(DATA_SOCK)

	ctrl_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	try:
		ctrl_sock.connect(CTRL_SOCK)
	except socket.error:
		print "ERROR: Cannot connect to recv server."
		sys.exit(1)
	ctrl_sock.send("subscribe %s" % DATA_SOCK)

	G = IsiFEngineGui(R, data_sock)
	G.start()


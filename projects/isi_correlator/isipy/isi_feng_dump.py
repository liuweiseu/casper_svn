#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np
import socket
import sys

from isi_data_view import IsiDataView

DATA_SOCK = "/tmp/isi_data_sock"
CTRL_SOCK = "/tmp/isi_ctrl_sock"
TIMESTAMP = False

datafmt = np.dtype \
([
	('pkt_id', '>i4'), 
	('adc', '>i4', 512),
	('pfb', '>i4', 128),
	('fftR', '>i4', 64),
	('fftI', '>i4', 64),
	('raR', '>i4', 64),
	('raI', '>i4', 64),
])

class IsiFengDump (IsiDataView):

	def __init__ (self):
		IsiDataView.__init__(self, datafmt)

	def view_data (self):
		print "Dumping data to file ..."
		self._dump_to_file("adc", self._DATA['adc'][0])
		self._dump_to_file("pfb", self._DATA['pfb'][0])
		self._dump_to_file("fftR", self._DATA['fftR'][0])
		self._dump_to_file("fftI", self._DATA['fftI'][0])
		self._dump_to_file("raR", self._DATA['raR'][0])
		self._dump_to_file("raI", self._DATA['raI'][0])

	def _dump_to_file (self, name, data):
		if TIMESTAMP:
			from time import strftime
			timestamp = strftime("%Y%m%dT%H%M%S")
			filename = "data/%s_%s.dump" % (timestamp, name)
		else:
			filename = "data/%s.dump" % (name)

		f = open(filename, "w")
		data.tofile(f, sep="\n", format="%d")
		f.close()

if __name__ == "__main__":
	IsiFengDump().main()


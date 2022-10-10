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
	('pb0', '>i4', 256),
])

class IsiTpbDump (IsiDataView):

	def __init__ (self):
		IsiDataView.__init__(self, datafmt)

	def view_data (self):
		print "Dumping data to file ..."
		self._dump_to_file("pb0", self._DATA['pb0'][0])

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
	IsiTpbDump().main()


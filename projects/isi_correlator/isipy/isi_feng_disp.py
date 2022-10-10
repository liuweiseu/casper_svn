#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np

from isi_data_view import IsiDataView

DATA_SOCK = "/tmp/isi_data_sock"
CTRL_SOCK = "/tmp/isi_ctrl_sock"

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

class IsiFengDisp (IsiDataView):

	def __init__ (self):
		IsiDataView.__init__(self, datafmt)

	def view_data (self):
		print "Packet #%d:" % self._DATA['pkt_id'][0]
		for i in xrange(64):
			print "%12d %12d %12d %12d %12d %12d" % (self._DATA['adc'][0][i], self._DATA['pfb'][0][i], self._DATA['fftR'][0][i], self._DATA['fftI'][0][i], self._DATA['raR'][0][i], self._DATA['raI'][0][i])

if __name__ == "__main__":
	IsiFengDisp().main()


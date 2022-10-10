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
ACCS_PER_PKT = 5

accfmt = np.dtype \
([
	('XX_auto', '>i4', 64),
	('YY_auto', '>i4', 64),
	('ZZ_auto', '>i4', 64),
	('XY_real', '>i4', 64),
	('XY_imag', '>i4', 64),
	('YZ_real', '>i4', 64),
	('YZ_imag', '>i4', 64),
	('ZX_real', '>i4', 64),
	('ZX_imag', '>i4', 64),
], copy=True)

datafmt = np.dtype \
([
	('pkt_id', '>i4'),
	('accums', accfmt, ACCS_PER_PKT),
])

class IsiCorrDisp (IsiDataView):

	def __init__ (self):
		IsiDataView.__init__(self, datafmt)

	def view_data (self):

		idx = 0
		print "Packet #%d:" % self._DATA['pkt_id'][0]
		print "%12s %12s %12s %12s %12s %12s %12s %12s %12s" % \
			('XX_auto', 'YY_auto', 'ZZ_auto', \
			 'XY_real', 'XY_imag', 'YZ_real', \
			 'YZ_imag', 'ZX_real', 'ZX_imag')
		for i in xrange(64):
			print "%12d %12d %12d %12d %12d %12d %12d %12d %12d" % \
				(self._DATA['accums'][0]['XX_auto'][idx][i],
				 self._DATA['accums'][0]['YY_auto'][idx][i],
				 self._DATA['accums'][0]['ZZ_auto'][idx][i],
				 self._DATA['accums'][0]['XY_real'][idx][i],
				 self._DATA['accums'][0]['XY_imag'][idx][i],
				 self._DATA['accums'][0]['YZ_real'][idx][i],
				 self._DATA['accums'][0]['YZ_imag'][idx][i],
				 self._DATA['accums'][0]['ZX_real'][idx][i],
				 self._DATA['accums'][0]['ZX_imag'][idx][i])

	def main (self):

		data_sock = self._open_data_client_sock(DATA_SOCK)
		ctrl_sock = self._open_ctrl_client_sock(CTRL_SOCK)
		ctrl_sock.send("subscribe %s" % DATA_SOCK)

		while True:
			data_sock.recv_into(self._DATA)
			self.view_data()

		self._close_data_client_sock(data_sock)
		self._close_ctrl_client_sock(ctrl_sock)

if __name__ == "__main__":
	IsiCorrDisp().main()


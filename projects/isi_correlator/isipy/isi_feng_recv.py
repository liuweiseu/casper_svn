#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np
import sys

from isi_data_recv import IsiDataRecv

pktfmt = np.dtype \
([  
	('pkt_id', '>i4'),
	('adc0', '>i4', 256),
	('adc1', '>i4', 256),
	('pfb0', '>i4', 64),
	('pfb1', '>i4', 64),
	('fftR', '>i4', 64),
	('fftI', '>i4', 64),
	('raR', '>i4', 64),
	('raI', '>i4', 64),
])

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

class IsiFengRecv (IsiDataRecv):

	def __init__ (self):
		IsiDataRecv.__init__(self, pktfmt, datafmt)

	def descramble (self):
		raw_adc0 = self._PKT['adc0'][0].reshape([8,32])
		raw_adc1 = self._PKT['adc1'][0].reshape([8,32])
		adc = np.row_stack((raw_adc0, raw_adc1)).transpose()

		raw_pfb0 = self._PKT['pfb0'][0].reshape([8,8])
		raw_pfb1 = self._PKT['pfb1'][0].reshape([8,8])
		pfb = np.row_stack((raw_pfb0, raw_pfb1)).transpose()

		fftR = self._PKT['fftR'][0].reshape([8,8]).transpose()
		fftI = self._PKT['fftI'][0].reshape([8,8]).transpose()

		raR = self._PKT['raR'][0].reshape([8,8]).transpose()
		raI = self._PKT['raI'][0].reshape([8,8]).transpose()

		self._DATA['pkt_id'] = self._PKT['pkt_id']
		self._DATA['adc'] = adc.flatten()
		self._DATA['pfb'] = pfb.flatten()
		self._DATA['fftR'] = fftR.flatten()
		self._DATA['fftI'] = fftI.flatten()
		self._DATA['raR'] = raR.flatten()
		self._DATA['raI'] = raI.flatten()

		return True

if __name__ == "__main__":
	IsiFengRecv().main()


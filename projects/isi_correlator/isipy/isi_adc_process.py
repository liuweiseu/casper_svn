#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np
import sys

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

import IPython
ipshell = IPython.Shell.IPShellEmbed()

num_boards = 2
num_samples = 256

datafmt = np.dtype \
([
	('pkt_id', '>i4'),
	('adc', '>i4', (num_boards, num_samples)),
])

def Plot2D (Y):
	if type(Y) == list:
		for y in Y:
			plt.plot(y)
	else:
		plt.plot(y)
	plt.show()

class IsiAdcProcess ():

	def __init__ (self, ifile):
		self._ifile = open(ifile, "r")

		self.main()

		self._ifile.close()

	def main (self):
		x = np.fromfile(self._ifile, dtype=datafmt)
		X = x['adc'][0]

		ipshell()

def main ():
	if len(sys.argv) == 1:
		print "Must specify an input file."
		return
	ifile = sys.argv.pop(1)

	IsiAdcProcess(ifile)

if __name__ == "__main__":
	main()


#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np
import sys

class IsiCorrTrans ():

	def __init__ (self, ifile, ofile):
		self._ifile = open(ifile, "r")
		self._ofile = open(ofile, "w")

		self.main()

		self._ofile.close()
		self._ifile.close()

	def main (self):
		x = np.fromfile(self._ifile, dtype='i4')
		y = x.reshape(-1,8,5,9,8)
		z = y.transpose(0,2,3,4,1)
		A = z.reshape(-1,9,64)

		tag = "Packet #%d:\n"
		fmt = "%12d" * 9 + "\n"

		for i in xrange(len(A)):
			self._ofile.write(tag % i)
			for j in xrange(64):
				self._ofile.write(fmt % \
				( \
					A[i][0][j], \
					A[i][1][j], \
					A[i][2][j], \
					A[i][3][j], \
					A[i][4][j], \
					A[i][5][j], \
					A[i][6][j], \
					A[i][7][j], \
					A[i][8][j], \
				))

def main ():
	sys.argv.pop(0)

	if len(sys.argv) == 0:
		print "Must specify an input file."
		return
	ifile = sys.argv.pop(0)

	if len(sys.argv) == 0:
		print "Must specify an outut file."
		return
	ofile = sys.argv.pop(0)

	IsiCorrTrans(ifile, ofile)

if __name__ == "__main__":
	main()


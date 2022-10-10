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

def Plot3D (Z):

	fig = plt.figure()
	ax = Axes3D(fig)

	x = np.arange(64)
	y = np.arange(Z.shape[1])
	X,Y = np.meshgrid(x,y)

	ax.plot_wireframe(X, Y, Z.transpose(), rstride=100, cstride=1)
	plt.show()

def Plot2D (Y):
	if type(Y) == list:
		for y in Y:
			plt.plot(y)
	else:
		plt.plot(y)
	plt.show()

class IsiCorrProcess ():

	def __init__ (self, ifile):
		self._ifile = open(ifile, "r")

		self.main()

		self._ifile.close()

	def main (self):
		x = np.fromfile(self._ifile, dtype='i4')
		y = x.reshape(-1,8,5,9,8)
		z = y.transpose(0,2,3,4,1)
		A = z.reshape(-1,9,64)

		B = A.transpose(1,2,0)

		XX_a = np.fft.fft(B[0])
		YY_a = np.fft.fft(B[1])
		ZZ_a = np.fft.fft(B[2])
		XY_r = np.fft.fft(B[3])
		XY_i = np.fft.fft(B[4])
		YZ_r = np.fft.fft(B[5])
		YZ_i = np.fft.fft(B[6])
		ZX_r = np.fft.fft(B[7])
		ZX_i = np.fft.fft(B[8])

		XX = np.abs(XX_a)
		YY = np.abs(YY_a)
		ZZ = np.abs(ZZ_a)
		XY = np.abs(XY_r + XY_i)
		YZ = np.abs(YZ_r + YZ_i)
		ZX = np.abs(ZX_r + ZX_i)

		ipshell()

def main ():
	if len(sys.argv) == 1:
		print "Must specify an input file."
		return
	ifile = sys.argv.pop(1)

	IsiCorrProcess(ifile)

if __name__ == "__main__":
	main()


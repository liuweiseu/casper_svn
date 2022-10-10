#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import numpy as np

def interleave_tvgs (A, num_tvgs, bram_size):
	"""Interleave ARRAY for use in NUM_TVGS TVGs of TVG_SIZE length.
	Loop this as many times as necessary to fill each TVG."""

	# Make sure we received an array.
	if type(A) != np.ndarray:
		print "ERROR: Need an array."
		return None

	# Make sure it's an int32 array.
	if type(A[0]) != np.int32:
		B = np.array(A, dtype=np.int32)
	else:
		B = A

	# Make sure it's not too long.
	num_vals = num_tvgs * bram_size
	if len(B) > num_vals:
		print "WARN: Too many TVG values! Truncating."
		C = B[:num_vals]
	else:
		C = B

	uniq_cols = len(C) / num_tvgs
	num_reps = bram_size / uniq_cols

	D = C.reshape((uniq_cols, num_tvgs))
	E = D.transpose()
	F = np.hstack([E]*num_reps)

	return F

def scale_32bit (array, coeff):
	"""Scale an array of BRAM data and round to int32."""
	z = np.zeros(len(array), dtype=np.int32)
	y = np.around(coeff*array, out=z)
	return y

def sine_wave (L, cycles=1, phase=0):
	"""Build an L-point sine wave with range [-1,1)."""
	x = np.arange(L)
	omega = 2*np.pi*cycles/L
	phi = 2*np.pi*phase
	y = np.sin(omega*x+phi)
	return y

def square_wave (L):
	"""Build an L-point square wave with range [-1,1]."""
	x0 = np.zeros(L/2)
	x1 = np.ones(L/2)
	y = np.concatenate((x0, x1))
	return y

def square_fs (L, N):
	"""Build an L-point square wave from N fourier coefficients."""
	X_ = np.arange(L)
	C_ = 2*np.arange(N)+1
	X = X_ * np.ones((N, L))
	C = C_ * np.ones((L, N))
	C = C.transpose()
	f = (1./C)*np.sin(C*2*np.pi*X/L)
	y = (4/np.pi) * np.sum(f, 0)
	return y

def triangle_wave (L):
	"""Build an L-point triangle wave with range [-1,1)."""
	s = L/4.
	s0 = np.arange(s)/s
	s1 = 1-np.arange(2*s)/s
	s2 = s0-1
	y = np.concatenate((s0, s1, s2))
	return y

def triangle_fs (L, N):
	"""Build an L-point triangle wave from N fourier coefficients."""
	X_ = np.arange(L)
	C_ = 2*np.arange(N)+1
	X = X_ * np.ones((N, L))
	C = C_ * np.ones((L, N))
	C = C.transpose()
	f = ((-1)**((C-1)/2.))*(1./C**2)*np.sin(C*2*np.pi*X/L)
	y = (8./(np.pi**2)) * np.sum(f, 0)
	return y

def sawtooth_wave (L):
	"""Build an L-point sawtooth wave with range [-1,1)."""
	y = (2./L)*np.arange(L)-1
	return y

def sawtooth_fs (L, N):
	"""Build an L-point sawtooth wave from N fourier coefficients."""
	X_ = np.arange(L)
	C_ = np.arange(N)+1
	X = X_ * np.ones((N, L))
	C = C_ * np.ones((L, N))
	C = C.transpose()
	f = (1./C)*np.sin(C*2*np.pi*X/L)
	y = (-2/np.pi) * np.sum(f, 0)
	return y

def random_noise (L, seed=None):
	"""Build an L-point random array with range [-1,1)."""
	np.random.seed(seed)
	y = 2*np.random.random(L)-1
	return y

def comb (L, period=None, offset=0):
	"""Build an L-point comb."""
	y = np.zeros(L)
	if period == None:
		period = L
	y[offset::period] = 1
	return y

def counter (L, period=None, offset=0, step=1):
	"""Build an L-point counter."""
	y = np.arange(L) * step + offset
	if period != None:
		y %= period
	return y

def main ():
	L = 8
	N = 100

	f0 = sine_wave(L)
	f1 = square_wave(L)
	f2 = square_fs(L, N)
	f3 = triangle_wave(L)
	f4 = triangle_fs(L, N)
	f5 = sawtooth_wave(L)
	f6 = sawtooth_fs(L, N)
	f7 = random_noise(L, seed=1234)
	f8 = comb(L, period=L/2)

	print "Sine:            %s" % (scale_32bit(f0))
	print "Square (pure):   %s" % (scale_32bit(f1))
	print "Square (fs):     %s" % (scale_32bit(f2))
	print "Triangle (pure): %s" % (scale_32bit(f3))
	print "Triangle (fs):   %s" % (scale_32bit(f4))
	print "Sawtooth (pure): %s" % (scale_32bit(f5))
	print "Sawtooth (fs):   %s" % (scale_32bit(f6))
	print "Random Noise:    %s" % (scale_32bit(f7))
	print "Comb:            %s" % (scale_32bit(f8))

if __name__ == "__main__":
	main()


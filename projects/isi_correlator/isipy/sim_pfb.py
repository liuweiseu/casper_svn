#!/usr/bin/env python

import pylab
from pylab import plot
import numpy as np

def sine_fxn (length, cycles, windows=1):
	assert (length > 0)
	assert (cycles > 0)
	assert (windows > 0)
	L = length * windows
	C = cycles * windows
	x = (2*np.pi) * np.arange(-L/2, L/2)/(float(L)/C)
	y = np.sin(x)
	return y

def pfb_fir (data, taps=4):
	assert (taps>0)
	length = len(data)
	x = np.arange(-length/2, length/2)/(float(length)/taps)
	s = np.sinc(x)
	w = np.hanning(length)
	A = s*w*data
	B = np.array_split(A, taps)
	C = np.array(B)
	D = C.sum(axis=0)
	return D

def fft_real (data):
	A = np.fft.rfft(data)
	B = np.fft.fftshift(A)
	C = np.abs(B)
	return C

def sim_pfbs (length, freq_res, taps):
	norm_L1 = np.zeros(freq_res)
	norm_L2 = np.zeros(freq_res)
	norm_inf = np.zeros(freq_res)
	res_step = float(length) / freq_res
	for i in xrange(freq_res):
		cycles = (i+.5) * res_step
		adc = sine_fxn(length, cycles, taps)
		pfb = pfb_fir(adc, taps)
		ADC = fft_real(adc[0:length])
		PFB = fft_real(pfb)
		diff = np.subtract(ADC, PFB)
		norm_L1[i] = np.linalg.norm(diff, 1)
		norm_L2[i] = np.linalg.norm(diff, 2)
		norm_inf[i] = np.linalg.norm(diff, inf)
	return (norm_L1, norm_L2, norm_inf)

avg_L1 = []
avg_L2 = []
avg_inf = []
for i in xrange(8):
	DATA = sim_pfbs (128, 1024, i+1)
	avg_L1 += [np.average(DATA[0])]
	avg_L2 += [np.average(DATA[1])]
	avg_inf += [np.average(DATA[2])]

adc = sine_fxn(128, np.pi, 4)
pfb = pfb_fir(adc, taps=4)
fft_raw = fft_real(adc[0:128])
fft_pfb = fft_real(pfb)

plot(adc)
plot(pfb)
plot(fft_raw)
plot(fft_pfb)


#!/opt/python/bin/python2.5
import corr, time, struct, os, pprint, random, pylab, sys
from numpy import *

#test change 2

spec_len = 64
def parse_spect_bram(bin_data, spec_len):
	spectrum = zeros(spec_len)
	for k in range(0, spec_len):
		new_coef = bin_data[4*k : 4*k+4]
		new_coef = new_coef[::-1]
		spectrum[k] = struct.unpack('I', new_coef)[0]
	return spectrum

def single_spect_data(fpga, bram_name):
	bram_bin_data = fpga.read(bram_name, spec_len*4, 0)
	spectrum = parse_spect_bram(bram_bin_data, spec_len)
	return spectrum
	
def plot_spect( fpga, plotfn = pylab.stem, bram_name = 'spectrum' ):
	spectrum = single_spect_data(fpga, bram_name)
	analog_freq = linspace(0, 2000, 64)
	plotfn( analog_freq, spectrum )
	
def read_raw_data_brams(fpga):
	data0_binstr = fpga.read('data0', 2048*4, 0)
	data1_binstr = fpga.read('data1', 2048*4, 0)
	data2_binstr = fpga.read('data2', 2048*4, 0)
	data3_binstr = fpga.read('data3', 2048*4, 0)
	
	return data0_binstr, data1_binstr, data2_binstr, data3_binstr

def parse_raw_data_brams( data0_binstr, data1_binstr, data2_binstr, data3_binstr):
	data0 = struct.unpack('b'*(2048*4), data0_binstr)
	data1 = struct.unpack('b'*(2048*4), data1_binstr)
	data2 = struct.unpack('b'*(2048*4), data2_binstr)
	data3 = struct.unpack('b'*(2048*4), data3_binstr)

	raw_data_merged = []
	# BRAM reads from the lowest address first, but word endian-ness is byte flipped
	for k in range(0,2048):
		# grab 1 word from each BRAM
		cur_data0_word = data0[4*k : 4*k+4]
		cur_data1_word = data1[4*k : 4*k+4]
		cur_data2_word = data2[4*k : 4*k+4]
		cur_data3_word = data3[4*k : 4*k+4]

		cur_data0_word = cur_data0_word[::-1]
		cur_data1_word = cur_data1_word[::-1]
		cur_data2_word = cur_data2_word[::-1]
		cur_data3_word = cur_data3_word[::-1]

		raw_data_merged += cur_data0_word + cur_data1_word + cur_data2_word + cur_data3_word
	return raw_data_merged
	
def reset_raw_capt(fpga):
	fpga.write_int('capt_rst', 1)
	fpga.write_int('capt_rst', 0)

def single_raw_capt( fpga ):
	reset_raw_capt(fpga)
	data0_binstr, data1_binstr, data2_binstr, data3_binstr, = read_raw_data_brams( fpga )
	resynth_data = parse_raw_data_brams( data0_binstr, data1_binstr, data2_binstr, data3_binstr )
	return resynth_data

def plot_raw_capt( fpga ):
	raw_data = single_raw_capt(fpga)
	pylab.plot ( range( len( raw_data ) ), raw_data)

def plot_all( fpga ):
	pylab.figure()
	pylab.subplot(2,1,1)
	plot_raw_capt(fpga)
	pylab.subplot(2,1,2)
	plot_spect(fpga, pylab.stem)	

if __name__ == '__main__':
	fpga = corr.katcp_wrapper.FpgaClient('192.168.0.2', 7147)
	time.sleep(5)
	if len(sys.argv) == 1:
		bitstream = 'spec.bof'
	else:
		bitstream = sys.argv[1]
	fpga.progdev(bitstream)

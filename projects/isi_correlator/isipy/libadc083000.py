#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import struct
import time

class adc083000 ():
	"""
	A high-level interface to a NatSemi ADC083000 board.
	"""

	CTRL_TRIG = 1<<0
	CTRL_ADC0 = 1<<1
	CTRL_ADC1 = 1<<2

	CONF_OE        = 1<<8
	CONF_OV        = 1<<9
	CONF_nDE       = 1<<10
	CONF_DCP       = 1<<11
	CONF_DCS       = 1<<12
	CONF_RTD       = 1<<13
	CONF_DRE       = 1<<14

	ADDR_CONFIG    = 0x1
	ADDR_OFFSET    = 0x2
	ADDR_AMPLITUDE = 0x3
	ADDR_PHASE_FIN = 0xd
	ADDR_PHASE_CRS = 0xe
	ADDR_TEST_PTRN = 0xf

	# CONFIG = 0x92ff @ POR
	DFLT_CONFIG    = 0xb2ff
	DFLT_OFFSET    = 0x007f
	DFLT_AMPLITUDE = 0x807f
	DFLT_PHASE_FIN = 0x007f
	DFLT_PHASE_CRS = 0x03ff
	DFLT_TEST_PTRN = 0xf7ff

	def __init__ (self, board, ctrl_reg="adc_ctrl_cmd"):
		self._board = board
		self._ctrl_reg = ctrl_reg

		self.reset_to_defaults()

	def _write (self, trig, addr, data):
		trig &= 0x07
		addr &= 0x0f
		data &= 0xffff

		trig ^= adc083000.CTRL_ADC0
		trig ^= adc083000.CTRL_ADC1
		trig |= adc083000.CTRL_TRIG

		cmd = (trig << 24) | (addr << 16) | (data)

		print "Writing cmd=%d." % cmd
		self._board.write_int(self._ctrl_reg, 0x0)
		self._board.write_int(self._ctrl_reg, cmd)

	def reset_to_defaults (self, adc=6):
		self._write(adc, adc083000.ADDR_CONFIG, adc083000.DFLT_CONFIG)
		self._write(adc, adc083000.ADDR_OFFSET, adc083000.DFLT_OFFSET)
		self._write(adc, adc083000.ADDR_AMPLITUDE, adc083000.DFLT_AMPLITUDE)
		self._write(adc, adc083000.ADDR_PHASE_FIN, adc083000.DFLT_PHASE_FIN)
		self._write(adc, adc083000.ADDR_PHASE_CRS, adc083000.DFLT_PHASE_CRS)
		self._write(adc, adc083000.ADDR_TEST_PTRN, adc083000.DFLT_TEST_PTRN)

	def set_config (self, state, adc=6):
		data = (0x007f & state) << 8 | 0x80ff
		self._write(adc, adc083000.ADDR_OFFSET, data)

	def set_offset (self, offset, adc=6):
		"""
		The input offset of the ADC is adjusted
		linearly and monotonically by this value.
		00h provides a nominal zero offset, while
		FFh provides a nominal 45 mV of offset.
		Thus, each code step provides 0.176 mV.
		"""

		if offset >= 0:
			value = offset
			sign = 0
		else:
			value = -offset
			sign = 1

		data = (0x00ff & value) << 8 | sign << 7 | 0x007f
		self._write(adc, adc083000.ADDR_OFFSET, data)

	def set_amplitude (self, amplitude, adc=6):
		"""
		The gain of the ADC is adjusted	linearly
		and monotonically with a 9 bit data value.
		Adjustment range is +/-20% of the nominal
		700 mVpp differential value.

		0   = 560 mVpp
		256 = 700 mVpp
		511 = 840 mVpp

		For best performance, this value should be
		limited to the range 192 to 448 (+/-15%).
		"""

		data = (0x01ff & amplitude) << 7 | 0x007f
		self._write(adc, adc083000.ADDR_AMPLITUDE, data)

	def set_phase_fine (self, phase, adc=6):
		"""
		The phase of the ADC clock is adjusted
		non-linearly, with a maximum of 110ps.
		"""

		data = (0x01ff & phase) << 7 | 0x007f
		self._write(adc, adc083000.ADDR_PHASE_FIN, data)

	def set_phase_coarse (self, phase, adc=6):
		print "Coarse phase adjust is unimplemented!"

	def set_test_pattern (self, state=True, adc=6):
		"""
		The ADC is disengaged and a test pattern
		generator is connected to the outputs.
		"""

		if state:
			data = 0xffff
		else:
			data = 0xf7ff

		self._write(adc, adc083000.ADDR_TEST_PTRN, data)


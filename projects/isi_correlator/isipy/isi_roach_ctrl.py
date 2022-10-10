#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

from any_roach_ctrl import AnyRoachCtrl
from libisiroach import IsiRoach

class IsiRoachCtrl (AnyRoachCtrl):

	prompt = "isi> "

	def __init__ (self):
		AnyRoachCtrl.__init__(self, IsiRoach)

	#
	# ISI Commands
	#

	def do_zero_ctrl (self, line):
		for id in self._ids:
			self._boards[id].zero_ctrl()
			print "[%d] Zeroed control register." % id

	def do_arm (self, line):
		for id in self._ids:
			self._boards[id].arm()
			print "[%d] Armed." % id

	def do_trig (self, line):
		for id in self._ids:
			self._boards[id].trig()
			print "[%d] Triggered." % id

	def do_reset (self, line):
		for id in self._ids:
			self._boards[id].reset()
			print "[%d] Reset." % id

	def do_control (self, line):
		for id in self._ids:
			val = self._boards[id].get_control()
			print "[%d] Control: 0x%08x" % (id, val)

	def do_status (self, line):
		for id in self._ids:
			val = self._boards[id].get_status()
			print "[%d] Status: 0x%08x" % (id, val)

if __name__ == '__main__':
	IsiRoachCtrl().cmdloop()


#!/usr/bin/env python

__author__ = "William Mallard"
__email__ = "wjm@llard.net"
__copyright__ = "Copyright 2010, CASPER"
__license__ = "GPL"
__status__ = "Development"

import socket
import struct
import time

target = ("localhost", 8880)

bytes = 3584
vals = [0] * (bytes/4)
fmt = '!I%dI' % len(vals)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.connect(target)

count = 0
while True:
	data = struct.pack(fmt, count, *vals)
	try:
		sock.send(data)
	except socket.error:
		sock.connect(target)
	count += 1
	time.sleep(1)


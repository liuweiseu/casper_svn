#!/bin/csh

set tape_drive = "/dev/rmt/1n"
set num_bufs = 0

# dd needs for blocksize (bs) to be a power of 2.
# dd will exit at end of file on a tape, even if
# the blocksize * count indicates to go further.
# So, we arrange the bs and count so that dd does
# hit eof and then we use the while loop to get
# as many files as we want.  In this context a file
# is 1 tape buffer which is 1024 x 1024 + 4096 bytes
# long.

while ($num_bufs < $1)
	dd bs=1048576 count=256 if=$tape_drive >> $2
	@ num_bufs++
end

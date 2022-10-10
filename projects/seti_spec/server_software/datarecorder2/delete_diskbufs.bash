#!/bin/bash

diskbufnum=0
nextfile=~/datarecorder2/diskbuf/diskbuf_ds0_${diskbufnum}.done


while [ 1 -eq 1 ]; do
    echo "checking if $nextfile exists"
	# get next done file
	if [ -e $nextfile ]; then
        	echo "deleting $nextfile"
        	rm $nextfile
        	diskbufnum=$[($diskbufnum+1)%4]
        	nextfile=~/datarecorder2/diskbuf/diskbuf_ds0_${diskbufnum}.done

	fi
    sleep 5
done


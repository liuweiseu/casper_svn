#!/bin/bash

diskbufnum=0
largefilenum=0

maxfilesize=524288000

nextfile=~/datarecorder2/diskbuf/diskbuf_ds0_${diskbufnum}.done
largefile=~/datarecorder2/diskbuf/largefile_$(date +%b%d%Y_%H%M)


while [ 1 -eq 1 ]; do
    echo "checking if $nextfile exists"
	# get next done file
	if [ -e $nextfile ]; then
        echo "cat $nextfile into $largefile"
		cat $nextfile >> $largefile
        rm $nextfile
        diskbufnum=$[($diskbufnum+1)%4]
        nextfile=~/datarecorder2/diskbuf/diskbuf_ds0_${diskbufnum}.done
        
        largefilesize=$(stat -c%s "$largefile")
        echo "largefilesize is $largefilesize"
        
        if [ $largefilesize -gt  $maxfilesize ]; then
            largefilenum=$[$largefilenum+1]
            largefile=~/datarecorder2/diskbuf/largefile_$(date +%b%d%Y_%H%M)
        fi
	fi
    sleep 5
done


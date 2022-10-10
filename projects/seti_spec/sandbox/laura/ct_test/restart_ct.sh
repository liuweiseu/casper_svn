#!/bin/bash

ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')
sleeptime=180 #seconds

if [ -n "$ctPN" ]
then
    kill -9 $ctPN
    sleep 1
fi

#Start bof file
/home/obs/laura/boffiles/seti_ct_fpga3_2009_Apr_12_1126.bof &
sleep $sleeptime

#Set TVG
/home/obs/laura/src/tvg/fill_ct_tvg.sh  1 > dum.txt
echo "TVG filled"
sleep 1
/home/obs/laura/src/tvg/set_ct_tvg.sh 511 1 1

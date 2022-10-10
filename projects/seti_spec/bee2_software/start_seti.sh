#!/bin/sh

dir=bin/

# kill any sendstatus jobs not started from pattern
echo "killing previous sendstatus"
sudo killall -q sendstatus.sh

echo "deleting previous nohup file"
rm nohup.out

echo "killing running bofs"
${dir}seti_kill.sh

echo "loading new bofs"
nohup ${dir}seti_start_pfb.sh &
nohup ${dir}seti_start_ct.sh &
nohup ${dir}seti_start_fft.sh &
nohup ${dir}seti_start_thr.sh &

sleep 10

echo "configuring chips"
${dir}conf_chip.sh

echo "done"


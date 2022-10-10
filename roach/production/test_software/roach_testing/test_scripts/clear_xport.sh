#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 test_directory"
  exit 1
fi
TESTDIR=$1
echo -n "Enter XPORT IP address (Default = 192.168.4.20): "
read IP_ADDRESS
if [ "$IP_ADDRESS" = "" ]; then
  IP_ADDRESS="192.168.4.20"
fi 
(cat $TESTDIR/xport_config/reset_data  | nc -u -p 30718 $IP_ADDRESS 30718) >& /dev/null &
sleep 4 
pkill -9 nc > /dev/null
#pkill -9 xportd
#echo "Wait for XPORTD network connection to close. This may take a few minutes..."
#while [ $(netstat | grep -c 10001) -gt 0 ]; do
#  sleep 10
#  echo "Connection still present...waiting for 10 seconds." | tee -a $LOGFILE
#done   
#echo "Connection closed." | tee -a $LOGFILE

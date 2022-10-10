#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 test_directory xport_ip_address"
  exit 1
fi
if [ ! $2 ]; then
  echo "error: usage $0 test_directory xport_ip_address"
  exit 1
fi

TESTDIR=$1
XPORTIP=$2
LOGFILE=$TESTDIR/log_files/rst_roach.log 
rm $LOGFILE &> /dev/null

echo -n "Resetting ROACH..." | tee $LOGFILE 
echo -n -e "\033\0377\0377\0377\0377\0377\0377\0377\0377" | nc -u -p 30704 $XPORTIP 30704 >> $LOGFILE 2>&1 &
disown %1
sleep 0.1
echo -n -e "\033\0377\0377\0377\0377\000\000\000\000" | nc -u -p 30704 $XPORTIP 30704 >> $LOGFILE 2>&1 &
echo "done." | tee $LOGFILE
disown %1
sleep 0.1
pkill -9 nc &> /dev/null

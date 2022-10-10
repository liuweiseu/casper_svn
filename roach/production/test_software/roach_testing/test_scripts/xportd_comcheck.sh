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

if ! netstat | grep -q "${XPORTIP}:10001     ESTABLISHED"; then
  while netstat | grep -q "${XPORTIP}:10001"; do
    if pgrep xportd; then
      pkill -9 xportd
    fi
    echo "Incorrect XPORTD connection state. Waiting 10 seconds for XPORTD connection to close..."
    sleep 10
  done  
  echo -n "XPORTD not running...starting..."
  $TESTDIR/roach_tools/xportd/xportd.exe $XPORTIP 10001 &> /dev/null &
  COUNTER=0
  while ! netstat | grep -q "${XPORTIP}:10001     ESTABLISHED"; do
    if [ $COUNTER -eq 5 ]; then
      echo "XPORTD did not start...exiting." 
      exit 1
    else
      let COUNTER=COUNTER+1
      sleep 1
    fi
  done   
  echo "done." 
fi
        

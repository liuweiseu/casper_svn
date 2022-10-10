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

echo -n "Checking XPORT connection..."
if ! ./xportd_comcheck.sh $TESTDIR $XPORTIP; then
  echo "error: connection to XPORT unsuccessful."
  exit 1
fi
echo "done."

echo -n "Resetting ROACH..."
($TESTDIR/roach_tools/xport_gpio/./xport_gpio.exe $XPORTIP $TESTDIR/roach_tools/xport_gpio/set_state_on) >> $LOGFILE 2>&1
($TESTDIR/roach_tools/xport_gpio/./xport_gpio.exe $XPORTIP $TESTDIR/roach_tools/xport_gpio/set_state_off) >> $LOGFILE 2>&1
echo "done."

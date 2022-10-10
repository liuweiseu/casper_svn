#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 test_directory stty XPORT_IP"
  exit 1
fi

if [ ! $2 ] ; then
  echo "usage: $0 test_directroy stty XPORT_IP"
  exit 1
fi
if [ ! $3 ] ; then
  echo "usage: $0 test_directroy stty XPORT_IP"
  exit 1
fi

TESTDIR=$1
XPORTIP=$3
LOGFILE=$TESTDIR/log_files/load_linux.log
rm $LOGFILE &> /dev/null
pkill -9 serial_ptys
rm /tmp/serial_in  >& /dev/null
rm /tmp/serial_out >& /dev/null
rm /tmp/serial_tmp >& /dev/null

$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out > /dev/null &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then
  
  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP >> $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP >> $LOGFILE

  cat /tmp/serial_out | tee /tmp/serial_tmp >> $LOGFILE &
  if ./find_string.sh /tmp/serial_tmp "login:" 20; then
    echo "Linux booted correctly." > /tmp/serial_in
  else
    echo "error: Linux did not boot correctly." | tee -a $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
else
  echo "error: serial_ttys exec failed"
  pkill -9 serial_ptys
  exit 1
fi

pkill -9 cat
pkill -9 serial_ptys


#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 test_directory stty XPORT_IP MAC_address"
  exit 1
fi

if [ ! $2 ] ; then
  echo "usage: $0 test_directroy stty XPORT_IP MAC_address"
  exit 1
fi

if [ ! $3 ] ; then
  echo "usage: $0 test_directroy stty XPORT_IP MAC_address"
  exit 1
fi

if [ ! $4 ] ; then
  echo "usage: $0 test_directroy stty XPORT_IP MAC_address"
  exit 1
fi

TESTDIR=$1
XPORTIP=$3
MACADDR=$4
LOGFILE=$TESTDIR/log_files/set_mac.log
rm $LOGFILE &> /dev/null
pkill -9 serial_ptys
pkill -9 dhcpd
rm /tmp/serial_in  >& /dev/null
rm /tmp/serial_out >& /dev/null
rm /tmp/serial_tmp >& /dev/null

$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out > /dev/null &
disown %1

sleep 1

if [ -L /tmp/serial_out ] ; then
  
  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP | tee -a $LOGFILE

  cat /tmp/serial_out | tee /tmp/serial_tmp >> $LOGFILE &
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 20; then
    echo "x" > /tmp/serial_in
  else
    echo "error: ROACH did not boot." | tee -a $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  sleep 1 
  echo -n "Clearing environment..." | tee -a $LOGFILE
  echo "run clearenv" > /tmp/serial_in
  sleep 5
  # Older versions of uBoot didn't have clearenv macro defined.
  if ./find_string.sh /tmp/serial_tmp 'Error: "clearenv" not defined' 1; then
    echo "erase FFF60000 FFF9FFFF" > /tmp/serial_in
    sleep 5
  fi
  echo "done"
  echo -n "Soft reset..."
  echo "reset" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 20; then
    echo "x" > /tmp/serial_in
    echo "done"
  else
    echo "error: ROACH did not boot uBoot." | tee -a $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  sleep 1
  echo -n "Setting MAC address..." | tee -a $LOGFILE
  echo "setenv ethaddr $MACADDR" > /tmp/serial_in
  sleep 1
  echo "saveenv" > /tmp/serial_in
  sleep 2
  echo "done." | tee -a $LOGFILE
  echo -n "Soft reset..." | tee -a $LOGFILE
  echo "reset" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 20; then
    echo "x" > /tmp/serial_in
  else
    echo "error: ROACH did not boot uBoot." | tee -a $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    pkill -9 dhcpd
    exit 1
  fi
  sleep 1
  echo "printenv" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "ethaddr=$MACADDR" 20; then
    echo "done." | tee -a $LOGFILE
  else
    echo "MAC address not set correctly...check MAC address format." | tee -a $LOGFILE
  fi

  echo -n "Setting PowerPC boot configuration..." | tee -a $LOGFILE
  echo "run init_eeprom" > /tmp/serial_in
  sleep 2
  echo "done."

else
  echo "error: serial_ttys exec failed"
  pkill -9 serial_ptys
  pkill -9 dhcpd
  exit 1
fi

pkill -9 dhcpd
pkill -9 cat
pkill -9 serial_ptys


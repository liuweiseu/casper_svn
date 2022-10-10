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
LOGFILE=$TESTDIR/log_files/run_bit.log
rm $LOGFILE &> /dev/null
pkill -9 serial_ptys
pkill -9 dhcpd
echo ""
echo "Restarting ROACH and running Built in Tests." | tee $LOGFILE
sleep 1
rm /tmp/serial_in  >& /dev/null
rm /tmp/serial_out >& /dev/null
rm /tmp/serial_tmp >& /dev/null

$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out > /dev/null &
SERIAL_PID=$!

sleep 1

if [ -L /tmp/serial_out ] ; then

  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  cat /tmp/serial_out | tee /tmp/serial_tmp | tee -a $LOGFILE &
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 20; then
    echo "x" > /tmp/serial_in
    echo "UBOOT boot successful."
  else
    echo "error: UBOOT did not boot." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    kill -9 $SERIAL_PID
    pkill -9 cat
    pkill -9 dhcpd
    exit 1
  fi
  echo ""
  echo ""
  echo "Waiting 20 seconds for network to come up..."
  sleep 20 
  echo -n "Starting DHCP server..." | tee -a $LOGFILE
  LEASEFILE=$TESTDIR/dhcpd/var/spool/dhcpd.leases
  rm $LEASEFILE &> $LOGFILE
  touch $LEASEFILE &> $LOGFILE
  (dhcpd -cf $TESTDIR/dhcpd/dhcpd_1g.conf -lf $LEASEFILE eth1) >> $LOGFILE 2>&1 &
  if ./find_string.sh $LOGFILE "Listening on Socket/eth1/192.168.3.0/24" 10; then
    echo "done." | tee -a $LOGFILE
    echo ""
    echo ""
  else
    echo "error: DHCPD did not start." | tee -a $LOGFILE
    kill -9 $SERIAL_PID
    exit 1
  fi

  echo "Running built in tests..." | tee -a $LOGFILE
  echo "run bit" > /tmp/serial_in
  
  if ./find_string.sh /tmp/serial_tmp "checks" 120; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: built in tests did not execute." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    kill -9 $SERIAL_PID
    pkill -9 cat
    pkill -9 dhcpd
    exit 1
  fi

else
  echo "error: serial_ttys exec failed"
  kill -9 $SERIAL_PID
  pkill -9 dhcpd
  exit 1
fi

pkill -9 dhcpd
pkill -9 cat
kill -9 $SERIAL_PID


#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 test_directory stty XPORT_IP"
  exit 1
fi

if [ ! $2 ] ; then
  echo "usage: $0 test_directory stty XPORT_IP"
  exit 1
fi

if [ ! $3 ] ; then
  echo "usage: $0 test_directory stty XPORT_IP"
  exit 1
fi

TESTDIR=$1
XPORTIP=$3
LOGFILE=$TESTDIR/log_files/burnin_test.log
NCLOGFILE=$TESTDIR/log_files/echotest.log
rm $LOGFILE &> /dev/null
rm $NCLOGFILE &> /dev/null

pkill -9 serial_ptys
pkill -9 cat
pkill -9 dhcpd
pkill -9 inetd
rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then

  cat /tmp/serial_out > /tmp/serial_tmp &
  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  disown %1
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 15; then
    echo "x" > /tmp/serial_in
    echo "UBOOT boot complete."
  else
    echo "error: UBOOT did not boot." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo -n "Booting Linux, will take about 20 seconds..."
  sleep 1
  echo "run soloboot" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "roach login:" 60; then
    echo "Linux boot successful." | tee -a $LOGFILE
  else
    echo "error: Linux boot not successful." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo -n "Setting ROACH IP address to 192.168.3.10..." | tee -a $LOGFILE
  echo "root" > /tmp/serial_in
  sleep 0.2 
  echo "/sbin/ifconfig eth0 192.168.3.10 netmask 255.255.255.0 up" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "eth0: link is up, 1000 FDX" 5; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: ROACH IP address not successfully set." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  
  # Save the serial interface log and restart
  cat /tmp/serial_tmp >> $LOGFILE
  pkill -9 serial_ptys
  pkill -9 cat
  sleep 0.3

  rm /tmp/serial_in &> /dev/null
  rm /tmp/serial_out &> /dev/null
  rm /tmp/serial_tmp &> /dev/null
  $TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out | tee -a $LOGFILE &
  disown %1
  sleep 1
  cat /tmp/serial_out | tee /tmp/serial_tmp &

  inetd -T
  echo ""
  echo -n "Enter size of echo network test in megabytes (transfer rate approx. 2 MB/s): " | tee -a $LOGFILE
  read ECHOTEST
  ECHOSIZE=$[$ECHOTEST*1000000]
  echo -n "Enter size of memory to test in megabytes: " | tee -a $LOGFILE
  read DDRTESTSIZE
  echo ""
  echo ""
  echo "The memory test and network echo test will now start. The echo test will end when the specified amount of data has been transferred and the memory test will loop indefinitely. Press enter to start the tests and enter again to stop and exit."
  read
  echo ""
  echo "memtester-ppc $DDRTESTSIZE" > /tmp/serial_in
  echo "?echotest 192.168.3.2 7 $ECHOSIZE" | nc 192.168.3.10 7147 | tee $NCLOGFILE &
  read

else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 nc
pkill -9 cat 
pkill -9 serial_ptys 


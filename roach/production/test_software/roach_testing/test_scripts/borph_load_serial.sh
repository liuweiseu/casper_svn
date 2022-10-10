#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 test_directory stty kernel_image initial_root_filesystem XPORT_IP"
  exit 1
fi

if [ ! $2 ] ; then
  echo "usage: $0 test_directory stty kernel_image initial_root_filesystem XPORT_IP"
  exit 1
fi

if [ ! $3 ] ; then
  echo "usage: $0 test_directory stty kernel_image initial_root_filesystem XPORT_IP"
  exit 1
fi

if [ ! $4 ] ; then
  echo "usage: $0 test_directory stty kernel_image initial_root_filesystem XPORT_IP"
  exit 1
fi

if [ ! $5 ] ; then
  echo "usage: $0 test_directory stty kernel_image initial_root_filesystem XPORT_IP"
  exit 1
fi

TESTDIR=$1
XPORTIP=$5
LOGFILE=$TESTDIR/log_files/borph_load.log
rm $LOGFILE &> /dev/null

pkill -9 serial_ptys
pkill -9 cat
sleep 1
rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then

  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP | tee -a $LOGFILE

  cat /tmp/serial_out > /tmp/serial_tmp &
  disown %1
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 15; then
    echo "x" > /tmp/serial_in
  else
    echo "error: ROACH did not boot." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi

  echo "Loading BORPH image and file system..." | tee -a $LOGFILE

  echo -n "Waiting for YMODEM transfer of kernel image ready..." | tee -a $LOGFILE
  echo "run yget" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "## Ready for binary (ymodem) download to 0x00200000 at 115200 bps..." 5; then
    echo "starting transfer." | tee -a $LOGFILE
  else
    echo "YMODEM transfer not ready...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 cat 
pkill -9 serial_ptys 

$TESTDIR/roach_tools/xyzmodem_tx/xyzmodem_tx $2 $3 /usr/local/bin/lsx.exe --ymodem # 2>&1 | tee -a $LOGFILE

pkill -9 lsx

rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then
  cat /tmp/serial_out > /tmp/serial_tmp &
  disown %1
  echo -n "Erasing flash from 0xfc000000 to 0xfc1bffff..." | tee -a $LOGFILE
  echo "era 0xfc000000 0xfc1bffff" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Erased 14 sectors" 60; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: erase not sucessfull...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo -n "Copying kernel image to flash..." | tee -a $LOGFILE
  echo "cp.b 0x200000 0xfc000000 0x1c0000" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Copy to Flash... done" 60; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: copy to flash not sucessfull...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 serial_ptys
pkill -9 cat


rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then
  cat /tmp/serial_out > /tmp/serial_tmp &
  disown %1
  echo -n "Waiting for YMODEM transfer of initial root file system ready..." | tee -a $LOGFILE
  echo "run yget" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "## Ready for binary (ymodem) download to 0x00200000 at 115200 bps..." 5; then
    echo "starting transfer." | tee -a $LOGFILE
  else
    echo "error: YMODEM transfer not ready...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 cat 
pkill -9 serial_ptys 

$TESTDIR/roach_tools/xyzmodem_tx/xyzmodem_tx $2 $4 /usr/local/bin/lsx.exe --ymodem # 2>&1 | tee -a $LOGFILE

pkill -9 lsx

rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then
  cat /tmp/serial_out > /tmp/serial_tmp &
  disown %1
  echo -n "Erasing flash from 0xfc200000 to 0xfc5fffff..." | tee -a $LOGFILE
  echo "era 0xfc200000 0xfc5fffff" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Erased 32 sectors" 60; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: erase not sucessfull...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo -n "Copying initial root file system to flash..." | tee -a $LOGFILE
  echo "cp.b 0x200000 0xfc200000 0x400000" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Copy to Flash... done" 60; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error: copy to flash not sucessfull...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo "reset" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "roach login:" 60; then
    echo "Linux boot successful." | tee -a $LOGFILE
  else
    echo "error: Linux boot not successful." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 serial_ptys
pkill -9 cat

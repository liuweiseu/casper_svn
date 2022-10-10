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
pkill -9 dhcpd
rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
rm /tmp/serial_tmp &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $2 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1
sleep 1

if [ -L /tmp/serial_out ] ; then

  cat /tmp/serial_out > /tmp/serial_tmp &

  echo "\n\nAttempting to load the Linux kernel and root filesystem to flash via TFTP transfer...\n\n"
  $TESTDIR/test_scripts/reset_roach.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  sleep 1
  $TESTDIR/test_scripts/roach_powerup.sh $TESTDIR $XPORTIP | tee -a $LOGFILE
  disown %1
  sleep 1
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 30; then
    echo "x" > /tmp/serial_in
    echo "UBOOT boot complete."
  else
    echo "error: UBOOT did not boot." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  echo ""
  echo "Waiting 10 seconds for network to come up..."
  sleep 10
  echo -n "Starting DHCP server..." | tee -a $LOGFILE
  LEASEFILE=$TESTDIR/dhcpd/var/spool/dhcpd.leases
  rm $LEASEFILE &> $LOGFILE
  touch $LEASEFILE &> $LOGFILE 
  (dhcpd -cf $TESTDIR/dhcpd/dhcpd_1g.conf -lf $LEASEFILE eth1) >> $LOGFILE 2>&1 & 
  if ./find_string.sh $LOGFILE "Listening on Socket/eth1/192.168.3.0/24" 10; then
    echo "done." | tee -a $LOGFILE
    echo ""
  else
    echo "error: DHCPD did not start." | tee -a $LOGFILE
    pkill -9 cat
    pkill -9 serial_ptys
    exit 1
  fi
  echo -n "Commanding ROACH to request an IP address..."
  echo "bootp" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "DHCP client bound to address 192.168.3.240" 120; then
    echo "IP address acquired." | tee -a $LOGFILE
  else
    echo "ROACH did not receive an IP address...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 dhcpd
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
  pkill -9 dhcpd
  
  echo -n "Starting TFTP transfer of kernel image..."
  sleep 2
  echo "tftp 0x00200000 192.168.3.2:$3" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Bytes transferred = 1465003" 15; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
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
  
  echo ""
  echo -n "Starting TFTP transfer of initial root file system..."
  echo "tftp 0x00200000 192.168.3.2:$4" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Bytes transferred = 3453952" 5; then
    echo "done." | tee -a $LOGFILE
  else
    echo "error...exiting." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
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
  COUNTER=0
  FOUND=0
  while [ $FOUND -eq 0 ]; do
    sleep 3
    if cat /tmp/serial_tmp | grep -a -B 1 "Copy to Flash... done" | grep -q "cp.b 0x200000 0xfc200000 0x400000"; then
      echo "done." | tee -a $LOGFILE
      FOUND=1
    elif [ $COUNTER -eq 60 ]; then
      echo "error: copy to flash not sucessfull...exiting." | tee -a $LOGFILE
      cat /tmp/serial_tmp >> $LOGFILE
      pkill -9 serial_ptys
      pkill -9 cat
      exit 1
    else  
      let COUNTER=COUNTER+1
    fi
  done

  echo ""
  echo -n "Booting Linux..."
  echo "reset" > /tmp/serial_in
  if ./find_string.sh /tmp/serial_tmp "Hit any key to stop autoboot:" 15; then
    echo "x" > /tmp/serial_in
    echo -n "UBOOT boot complete..."
  else
    echo "error: UBOOT did not boot." | tee -a $LOGFILE
    cat /tmp/serial_tmp >> $LOGFILE
    pkill -9 serial_ptys
    pkill -9 cat
    exit 1
  fi
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
  echo ""

else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 serial_ptys
  exit 1
fi

cat /tmp/serial_tmp >> $LOGFILE
pkill -9 cat 
pkill -9 serial_ptys 


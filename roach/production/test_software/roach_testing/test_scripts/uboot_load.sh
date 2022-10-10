#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 test_directory mac_file stty uboot.bin XPORT_IP"
  exit 1
fi

if [ ! $2 ] ; then
  echo "usage: $0 test_directory mac_file stty uboot.bin XPORT_IP"
  exit 1
fi

if [ ! $3 ] ; then
  echo "usage: $0 test_directory mac_file stty uboot.bin XPORT_IP"
  exit 1
fi

if [ ! $4 ] ; then
  echo "usage: $0 test_directory mac_file stty uboot.bin XPORT_IP"
  exit 1
fi

if [ ! $5 ] ; then
  echo "usage: $0 test_directory mac_file stty uboot.bin XPORT_IP"
  exit 1
fi

TESTDIR=$1
LOGFILE=$TESTDIR/log_files/uboot_load.log
rm $LOGFILE &> /dev/null
MAC_DIR=`(echo $2 | grep -q "/" && echo $2 | sed 's#[^/]*$##g') || echo "./"`
MAC_FILE=`echo $2 | sed 's#^.*/##g'`

OCD_BIN=ocd_cmdl

echo ""
echo -n "Loading macro \"$MAC_FILE\"..." | tee $LOGFILE
cd ${MAC_DIR}
${OCD_BIN} ${MAC_FILE} &
disown %1
cd - &> /dev/null
# this delay is required to wait for OCD commander to load
sleep 4
echo "done." | tee -a $LOGFILE

pkill -9 serial_ptys
sleep 1
rm /tmp/serial_in &> /dev/null
rm /tmp/serial_out &> /dev/null
$TESTDIR/roach_tools/serial_ptys/serial_ptys $3 /tmp/serial_in /tmp/serial_out >> $LOGFILE 2>&1 &
disown %1

sleep 1

if [ -L /tmp/serial_out ] ; then
  echo -n "Waiting for XMODEM transfer ready..." | tee -a $LOGFILE
  $TESTDIR/roach_tools/charfind/charfind "\025" < /tmp/serial_out
else
  echo "error: serial_ttys exec failed" | tee -a $LOGFILE
  pkill -9 Ocd_cmdl
  pkill -9 serial_ptys
  exit 1
fi

echo "got XMODEM transfer ready." | tee -a $LOGFILE
pkill -9 Ocd_cmdl 
pkill -9 serial_ptys 

($TESTDIR/roach_tools/xyzmodem_tx/xyzmodem_tx $3 $4 /usr/local/bin/lsx.exe) # 2>&1 | tee -a $LOGFILE

#pkill -9 lsx
#echo ""
#echo -n "Enter MAC address (xx:xx:xx:xx:xx:xx): "
#read MACADDR
#./set_mac.sh $TESTDIR /dev/com1 $5 $MACADDR 2>&1 | tee -a $LOGFILE


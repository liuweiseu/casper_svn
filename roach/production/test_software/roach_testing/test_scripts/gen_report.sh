#!/bin/bash

if [ ! $1 ]; then
  echo "error: usage $0 test_directory"
  exit 1
fi

TESTDIR=$1
LOGFILE=$TESTDIR/log_files/roach_init.log

echo ""
echo "*******************************************************************"
echo "***  Test report (see section above for built in test results). ***"
echo "*******************************************************************"
echo ""


if cat $LOGFILE | grep -q "Checking XPORT connection...done." && cat $LOGFILE | grep -q "Resetting ROACH...done." && cat $LOGFILE | grep -q "Powering-up ROACH...done."; then
  echo "ROACH reset and power-up successful."
else
  echo "error: ROACH did not reset or power-up correctly."
fi

if cat $LOGFILE | grep -q "Programming STAPL file...done." && cat $LOGFILE | grep -q "Clearing Actel non volatile memory...done." && cat $LOGFILE | grep -q "Adding serial number to ufc file...done." && cat $LOGFILE | grep -q "Programming FPGA ROM...done."; then
  echo "ROACH monitor Actel configured correctly."
else
  echo "error: ROACH monitor Actel not configured correctly."
fi

if cat $LOGFILE | grep -q "roach_cpld_program\"...done." || cat $LOGFILE | grep -q "roach_02_cpld_program\"...done."; then
  echo "CPLD programmed successfully."
else
  echo "error: CPLD not programmed correctly."
fi

if cat $LOGFILE | grep -q "got XMODEM transfer ready." && cat $LOGFILE | grep -q "Bytes Sent: 393216"; then
  echo "UBOOT load successful."
else
  echo "error: UBOOT did not load correctly."
fi

if grep -q "Copying kernel image to flash...done." $LOGFILE; then
  echo "BORPH kernel image load successful."
else
  echo "error: BORPH kernel image did not load correctly."
fi

if grep -q "Copying initial root file system to flash...done." $LOGFILE; then
  echo "BORPH root file system load successful."
else
  echo "error: BORPH root file system did not load correctly."
fi

if cat $LOGFILE | grep -q "Linux boot successful."; then
  echo "Linux with BORPH loaded and running successfully."
else
  echo "error: Linux did not load correctly."
fi

if cat $LOGFILE | grep -q "Setting MAC address...done."; then
  echo "MAC address correctly set."
else
  echo "error: MAC address not correctly set...check MAC address format."
fi


#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 test_directory serialnumber XPORT_IP"
  exit 1
fi
if [ ! $2 ]; then
  echo "error: usage $0 test_directory serialnumber XPORT_IP"
  exit 1
fi
if [ ! $3 ]; then
  echo "error: usage $0 test_directory serialnumber XPORT_IP"
  exit 1
fi

XPORTIP=$3
TESTDIR=$1
LOGFILE=$TESTDIR/log_files/config_roach_monitor.log
rm $LOGFILE &> /dev/null

if ! echo $2 | grep -q '^\w\w\w\w\w\w$' ; then
  echo "Invalid serial number" | tee $LOGFILE
  exit 1
fi
SERIAL=$2

#Program ufc file
echo -n "Programming STAPL file..." | tee -a $LOGFILE
cd $TESTDIR/flashpro/fpga_program
cp roach_monitor.stp fpga_program.stp
rm -fr deleteme &> /dev/null
flashpro script:fpga_program.tcl logfile:fpga_program.log >> $LOGFILE 2>&1
cd - > /dev/null
if cat $TESTDIR/flashpro/fpga_program/fpga_program.log | grep -q "Executing action PROGRAM PASSED."; then
  echo "done." | tee -a $LOGFILE
else
  echo "programming error!"
fi

#Clear NVM
echo -n "Clearing Actel non volatile memory..." | tee -a $LOGFILE
./roach_mon_set_nvm.sh $TESTDIR $XPORTIP >> $LOGFILE 2>&1
echo "done."  | tee -a $LOGFILE

#Configure ufc file with serial number
echo -n "Adding serial number to ufc file..." | tee -a $LOGFILE
./serial2rom.sh $SERIAL $TESTDIR/gen_files/roach_monitor/roach_monitor.ufc $TESTDIR/flashpro/from_program/roach_monitor.ufc >> $LOGFILE 2>&1
echo "done." | tee -a $LOGFILE

#Program ufc file
echo -n "Programming FPGA ROM..." | tee -a $LOGFILE
cd $TESTDIR/flashpro/from_program
rm -fr deleteme &> /dev/null
flashpro script:from_program.tcl logfile:from_program.log >> $LOGFILE 2>&1
cd - > /dev/null
if cat $TESTDIR/flashpro/from_program/from_program.log | grep -q "Executing action PROGRAM PASSED."; then
  echo "done." | tee -a $LOGFILE
  echo -n "Power needs to be hard-cycled now. Turn power off, wait 10sec, power back on and then press enter."
  read
else
  echo "programming error!"
fi

#pkill -9 xportd
#echo ""

#while netstat | grep -q "${XPORTIP}:10001"; do
#  echo "Waiting 10 seconds for XPORTD connection to close..."
#  sleep 10
#done

echo ""

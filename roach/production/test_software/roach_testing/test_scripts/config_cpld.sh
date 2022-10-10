#!/bin/bash

if [ ! $1 ]; then
  echo "error: usage $0 test_directory ROACH_serial_number"
  exit 1
fi
if [ ! $2 ]; then
  echo "error: usage $0 test_directory ROACH_serial_number"
  exit 1
fi
if ! echo $2 | grep -q '^[[:digit:]]\{6\}$' ; then
  echo "Invalid serial number, must be a 6 digit number."
  exit 1
fi

TESTDIR=$1
SERIAL=$2
LOGFILE=$TESTDIR/log_files/config_cpld.log
rm $LOGFILE &> /dev/null


VERSION=${SERIAL:0:2}
WINPATH=$(cygpath -wl $TESTDIR/gen_files/cpld/roach_cpld.jed | sed -e 's%\\%\\\\%g')
echo "" 

#if [ $VERSION == "02" ]; then
  echo -n "ROACH version 02, CPLD 2'nd device in JTAG chain, using programming script: \"roach_02_cpld_program\"..." | tee -a $LOGFILE
  roach_02_cpld_program $WINPATH >> $LOGFILE 2>&1
  cat $LOGFILE | grep -q "ERROR"
  PROGRAM_ERROR=$?
  cat $LOGFILE | grep -q "Programming completed successfully"
  PROG_SUCCESS=$?
  if ! grep -q "Cable PID" $LOGFILE && ! grep -q "Cable connection established." $LOGFILE; then
    echo "cable error...check log file."
    exit 1
  elif [ $PROGRAM_ERROR -eq 0 ]; then
    echo "programming error...check log file."
    exit 1
  elif [ $PROG_SUCCESS -eq 0 ]; then
    echo "done." | tee -a $LOGFILE
  else
    echo "programming unsuccessfull...check log file."
  fi
#elif [ $VERSION == "01" ]; then
#  echo -n "ROACH version 01, CPLD on seperate JTAG chain, using programming script \"roach_cpld_program\"..." | tee -a $LOGFILE
#  roach_cpld_program $WINPATH >> $LOGFILE 2>&1
#  cat $LOGFILE | grep -q "ERROR"
#  PROGRAM_ERROR=$?
#  cat $LOGFILE | grep -q "Programming completed successfully"
#  PROG_SUCCESS=$?
#  if ! grep -q "Cable PID" $LOGFILE && ! grep -q "Cable connection established." $LOGFILE; then
#    echo "cable error...check log file."
#    exit 1
#  elif [ $PROGRAM_ERROR -eq 0 ]; then
#    echo "programming error...check log file."
#    exit 1
#  elif [ $PROG_SUCCESS -eq 0 ]; then
#    echo "done." | tee -a $LOGFILE
#  else
#    echo "programming unsuccessfull...check log file."
#  fi
#else
#  echo "Unknown ROACH version...exiting" | tee -a $LOGFILE
#  exit 1
#fi

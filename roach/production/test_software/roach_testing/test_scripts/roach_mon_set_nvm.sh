#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 test_directory xport_ip_address"
  exit 1
fi
if [ ! $2 ]; then
  echo "error: usage $0 test_directory xport_ip_address"
  exit 1
fi

TESTDIR=$1
XPORTIP=$2

echo -n "Checking XPORT connection..."
if ! ./xportd_comcheck.sh $TESTDIR $XPORTIP &> /dev/null; then
  echo "error: connection to XPORT unsuccessful."
  exit 1
fi
echo "done."

$TESTDIR/roach_tools/monman/roach_test_scripts/roach_monitor/./run_mm $TESTDIR/roach_tools/monman/roach_test_scripts/roach_monitor/sysconfig_clear.cmnd

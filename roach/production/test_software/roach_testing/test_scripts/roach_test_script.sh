#!/bin/bash
TESTDIR=/cygdrive/c/roach_testing
DEF_SERIAL=010001
SERIAL=010001
XPORTIP=192.168.4.20
PS3="Serial number set to: ${SERIAL}
Press enter to see menu > "

cd $TESTDIR/test_scripts/

pkill -9 xportd
pkill -9 serial_ptys
pkill -9 cat
pkill -9 dhcpd

OPTIONS=("Initialise ROACH and run automated tests" "Clear XPort" "Configure XPort" "Set ROACH serial number" "Configure Actel Fusion (ROACH Monitor) and save serial number" "Configure CPLD" "Load UBOOT Image to Flash" "Set ROACH MAC address, clear uBoot environment and save PPC boot configuration" "Load Linux (BORPH) kernel (uImage) and root filesystem (romfs, mini-root) to Flash" "Run Built in Tests" "Run ROACH burn in tests" "Reset ROACH" "Power-up ROACH" "Quit")
select opt in "${OPTIONS[@]}"; do
  case $opt in
    "Initialise ROACH and run automated tests")
      LOGFILE=$TESTDIR/log_files/roach_init.log
      rm $TESTDIR/log_files/*.log &> /dev/null

      echo -n "Enter a 6 digit serial number: "
      read SERIAL
      if ! echo $SERIAL | grep -q '^[[:digit:]]\{6\}$'; then
        SERIAL=$DEF_SERIAL
        echo "Incorrect serial number format."
      else
        MACADDR="02:00:00:${SERIAL:0:2}:${SERIAL:2:2}:${SERIAL:4:2}"
        echo "MAC addr will default to $MACADDR"
        PS3="Serial number set to: ${SERIAL}
Press enter to see menu > "
      fi

      echo "Clearing any XPort at $XPORTIP..."
      (cat $TESTDIR/xport_config/reset_data  | nc -u -p 30718 $XPORTIP 30718) >& /dev/null &
      sleep 4
      pkill -9 nc > /dev/null

      ./config_xport.sh $TESTDIR 2>&1 | tee -a $LOGFILE
      sleep 1
      ./config_roach_monitor.sh $TESTDIR $SERIAL $XPORTIP 2>&1 | tee -a $LOGFILE
      pkill -9 xportd
      echo "Waiting 10seconds for XPort to startup..."
      sleep 10
      ./reset_roach.sh $TESTDIR $XPORTIP  2>&1 | tee -a $LOGFILE
      sleep 1
      ./roach_powerup.sh $TESTDIR $XPORTIP 2>&1 | tee -a $LOGFILE
      sleep 1
      ./config_cpld.sh $TESTDIR $SERIAL 2>&1 | tee -a $LOGFILE
      sleep 1
      ./reset_roach.sh $TESTDIR $XPORTIP  2>&1 | tee -a $LOGFILE
      sleep 1
      ./roach_powerup.sh $TESTDIR $XPORTIP 2>&1 | tee -a $LOGFILE
      sleep 1
      ./uboot_load.sh $TESTDIR $TESTDIR/macraigor_macros/loadram_rinit_auto.mac /dev/com1 $TESTDIR/gen_files/uboot/u-boot.bin $XPORTIP 2>&1 | tee -a $LOGFILE
      sleep 4
      ./set_mac.sh $TESTDIR /dev/com1 $XPORTIP $MACADDR | tee -a $LOGFILE
      sleep 4
      ./borph_load_tftp.sh $TESTDIR /dev/com1 uImage romfs $XPORTIP 2>&1 | tee -a $LOGFILE
      sleep 4 
      ./run_bit.sh $TESTDIR /dev/com1 $XPORTIP 2>&1 | tee -a $LOGFILE
      ./gen_report.sh $TESTDIR | tee -a $LOGFILE
      tar czvf $TESTDIR/log_files/$SERIAL.tar.gz $TESTDIR/log_files/*.log &> /dev/null
      ;;
    "Set ROACH serial number")
      echo -n "Enter a 6 digit serial number: "
      read SERIAL
      if ! echo $SERIAL | grep -q '^[[:digit:]]\{6\}$'; then
        SERIAL=$DEF_SERIAL
        echo "Incorrect serial number format."
      else
        PS3="Serial number set to: ${SERIAL}
Press enter to see menu > "
      fi
      ;;
    "Configure XPort")
      ./config_xport.sh $TESTDIR
      ;;
    "Clear XPort")
      ./clear_xport.sh $TESTDIR
      ;;  
    "Configure Actel Fusion (ROACH Monitor) and save serial number")
      ./config_roach_monitor.sh $TESTDIR $SERIAL $XPORTIP
      echo "Waiting 10seconds for XPort to startup..."
      sleep 10
      ./reset_roach.sh $TESTDIR $XPORTIP  2>&1 | tee -a $LOGFILE
      sleep 1
      ./roach_powerup.sh $TESTDIR $XPORTIP 2>&1 | tee -a $LOGFILE
      ;;
    "Configure CPLD")
      ./roach_powerup.sh $TESTDIR $XPORTIP
      sleep 2
      ./config_cpld.sh $TESTDIR $SERIAL
      ./reset_roach.sh $TESTDIR $XPORTIP
      sleep 2
      ./roach_powerup.sh $TESTDIR $XPORTIP
      ;;
    "Load UBOOT Image to Flash")
      ./roach_powerup.sh $TESTDIR $XPORTIP
      sleep 2
      ./uboot_load.sh $TESTDIR $TESTDIR/macraigor_macros/loadram_rinit_auto.mac /dev/com1 $TESTDIR/gen_files/uboot/u-boot.bin $XPORTIP
      ;;
    "Load Linux (BORPH) kernel (uImage) and root filesystem (romfs, mini-root) to Flash")
      ./borph_load_tftp.sh $TESTDIR /dev/com1 uImage romfs $XPORTIP
      ;;
    "Run Built in Tests") 
      ./run_bit.sh $TESTDIR /dev/com1 $XPORTIP
      ;;
    "Set ROACH MAC address, clear uBoot environment and save PPC boot configuration")
      echo -n "Enter MAC address (xx:xx:xx:xx:xx:xx): "
      read MACADDR
      ./set_mac.sh $TESTDIR /dev/com1 $XPORTIP $MACADDR
      ./reset_roach.sh $TESTDIR $XPORTIP
      ;;
    "Reset ROACH")
      ./reset_roach.sh $TESTDIR $XPORTIP 
      sleep 2
      ./roach_powerup.sh $TESTDIR $XPORTIP 
      ;;
    "Power-up ROACH")
      ./roach_powerup.sh $TESTDIR $XPORTIP
      ;;
    "Run ROACH burn in tests")
      ./burnin_test.sh $TESTDIR /dev/com1 $XPORTIP
      ;;
    "Quit")
      pkill -9 xportd
      pkill -9 serial_ptys
      pkill -9 cat
      pkill -9 dhcpd
      exit
      ;;
  esac 
done

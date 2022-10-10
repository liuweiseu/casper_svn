#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 test_directory"
  exit 1
fi
TESTDIR=$1
rm $TESTDIR/log_files/config_xport.log &> /dev/null
#XPORT Configuration
    #Start DHCPD server
    LEASEFILE=$TESTDIR/dhcpd/var/spool/dhcpd.leases 
    rm $LEASEFILE &> /dev/null 
    touch $LEASEFILE &> /dev/null 
    (dhcpd -cf $TESTDIR/dhcpd/dhcpd_xport.conf -lf $LEASEFILE eth2) > $TESTDIR/log_files/config_xport.log 2>&1 &
    echo ""
    echo "1. Plug in ATX power supply and switch on. Verify 3.3V auxiliary voltage (Q11 - pin 2) and 1.5V auxiliary voltage (Q11 - pin 3)."
    echo ""
    echo "2. Connect the XPORT to the network card marked "XPORT". This card should be configured with the following static IP address: 192.168.4.2"
    echo ""
    echo "Press enter when ready"
    read
    echo ""

    #Ping checks
    echo "Waiting for response from XPORT... NOTE: The XPORT will send out DHCP request for 11 seconds after powerup and then every 5 minutes. If a ping is not received from the device try power cycling the board at the wall. Wait for the capacitors to discharge before switching back on, the XPORT link light will go out."
    echo ""
    COUNTER=0
    PING_TO_CNT=10
    while [ $COUNTER -lt $PING_TO_CNT ]; do
      ping 192.168.4.240 192 1 | grep -q "bytes from"
      if [ $? -eq 0 ]; then
        COUNTER=$PING_TO_CNT
        pkill -9 dhcpd
        echo "Response received... configuring XPORT." | tee -a $TESTDIR/log_files/config_xport.log
      elif [ $COUNTER -eq $[PING_TO_CNT-1] ]; then
        echo XPORT did not receive an IP address. Check network cable. | tee -a $TESTDIR/log_files/config_xport.log
        pkill -9 dhcpd
        exit
      else
        let COUNTER=$[COUNTER + 1] 
      fi
    done  
    echo -n "Sending setup record 0..."
    (echo -n -e "\000\000\000\0375"; cat $TESTDIR/xport_config/record0) | nc -w 1 -u -p 30718 192.168.4.240 30718 >> $TESTDIR/log_files/config_xport.log 2>&1
    echo "done."
    echo "Waiting 5 seconds for XPORT to reboot..."
    sleep 5
    #Send configuration data to XPORT port 0x77FE 
    #Send setup record 0
    echo -n "Sending setup record 7..."
    (echo -n -e "\000\000\000\0307"; cat $TESTDIR/xport_config/record7) | nc -w 1 -u -p 30718 192.168.4.20 30718 >> $TESTDIR/log_files/config_xport.log 2>&1
    echo "done."
    echo "Waiting 5 seconds for XPORT to reboot..."
    sleep 5
    echo -n "Sending setup record 3..."
    (echo -n -e "\000\000\000\0303"; cat $TESTDIR/xport_config/record3) | nc -w 1 -u -p 30718 192.168.4.20 30718 >> $TESTDIR/log_files/config_xport.log 2>&1
    echo "done."
    echo ""

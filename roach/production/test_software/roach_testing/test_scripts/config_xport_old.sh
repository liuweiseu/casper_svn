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
        echo -n "Response received... configuring XPORT..." | tee -a $TESTDIR/log_files/config_xport.log
      elif [ $COUNTER -eq $[PING_TO_CNT-1] ]; then
        echo XPORT did not receive an IP address. Check network cable. | tee -a $TESTDIR/log_files/config_xport.log
        pkill -9 dhcpd
        exit
      else
        let COUNTER=$[COUNTER + 1] 
      fi
    done  
    (cat $TESTDIR/xport_config/setup_data | nc -w 1 -u -p 30718 192.168.4.240 30718) >> $TESTDIR/log_files/config_xport.log 2>&1
    echo "done."
    echo "Waiting 5 seconds for XPORT to reboot..."
    sleep 5
    #Send configuration data to XPORT port 0x77FE 
    #Retry 3 times
    COUNTER=0
    CONFIG_ERR=1
    echo -n "Configuring XPORT GPIOs..."
    while [ $COUNTER -lt 3 ] && [ $CONFIG_ERR -eq 1 ]; do
      #Configure XPORT GPIO
      (echo -e -n "M"; sleep 1; echo -e -n "S7\r:20000010000701808000000000008000000000000000000000000000000000000000000048:200020100000000000000000000000000000000000000000000000000000000000000000B0:20004010000000000000000000000000000000000000000000000000000000000000000090:1E00601000000000000000000000000000000000000000000000000000000000000072:00000001FF"; echo -e -n "RS\r") | nc -w 1 192.168.4.20 9999 > /tmp/test
      if ! grep -q "0>S7" /tmp/test ; then 
        if [ $COUNTER -eq 2 ]; then
          echo "error: GPIO configuration failed."
          exit 1
        else
          echo -n "configuration failed...retrying..."
        fi  
      else 
        echo "GPIO configuration done."
        CONFIG_ERR=0
      fi
      let COUNTER=COUNTER+1
      sleep 5
    done  
    #Configure XPORT High performance mode
    COUNTER=0
    CONFIG_ERR=1
    echo -n "Configuring XPORT High Performance Mode..."
    while [ $COUNTER -lt 3 ] && [ $CONFIG_ERR -eq 1 ]; do
      echo -e -n "\r5\r\r\rY\r\r\r\r\r\r9\r" | nc -t -w 1 192.168.4.20 9999 > /tmp/test
      if ! grep -q "Parameters stored" /tmp/test; then 
        if [ $COUNTER -eq 2 ]; then
          echo "error: XPORT High Performance Mode configuration failed"
        else
          echo -n "configuration failed...retrying..."
          exit 1
        fi  
      else 
        echo "High Performance configuration done." 
        CONFIG_ERR=0
      fi
      let COUNTER=COUNTER+1
      sleep 5
    done  
    echo ""

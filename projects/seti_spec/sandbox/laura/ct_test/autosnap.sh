zeros=1
timestart=$(date +%s)
timenow=$timestart
logname=$timestart
timestep=$((2*60))         #In seconds
timedelay=$((2*60))
loopnum=60

path=/home/obs/laura
loopvar=0
ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')

echo $(date)

#Grab first SNAP block
./init_ct_test.sh "0_$timestart"

#Check SNAP addresses
addr_in=$( $path/regread.sh ct sys_snap_in_addr | awk '{print $3}')
echo "addr_in: $addr_in"
addr_mid=$( $path/regread.sh ct sys_snap_mid_addr | awk '{print $3}')
echo "addr_mid: $addr_mid"
addr_out=$( $path/regread.sh ct sys_snap_out_addr | awk '{print $3}')
echo "addr_out: $addr_out"

#Check sync registers
sync0o=$( $path/regread.sh ct sys_dram_ctrl_sync_check0 | awk '{print $2 $3}')
echo "sync0: $sync0o"
sync1o=$( $path/regread.sh ct sys_dram_ctrl_sync_check1 | awk '{print $2 $3}')
echo "sync1: $sync1o"
sync2o=$( $path/regread.sh ct sys_dram_ctrl_sync_check2| awk '{print $2 $3}')
echo "sync2: $sync2o"
sync3o=$( $path/regread.sh ct sys_dram_ctrl_sync_check3| awk '{print $2 $3}')
echo "sync3: $sync3o"
sync4o=$( $path/regread.sh ct sys_dram_ctrl_sync_check4| awk '{print $2 $3}')
echo "sync4: $sync4o"

#Begin logging
echo $timestart $(date) $timedelay $addr_in $addr_mid $addr_out $((0x$sync0o)) $((0x$sync1o)) $((0x$sync2o)) $((0x$sync3o)) $((0x$sync4o)) > "log_$logname.txt"

#if [[ "$addr_in" == "0000" || "$addr_mid" == "0000" || "$addr_out" == "0000" ]]
#then
#    zeros=0
#fi

#Delay loop
    while [ $timenow -le $((timestart + timedelay)) ]
    do
        sleep 1
        timenow=$(date +%s)
    done    

#Begin loop that checks for constants
#while [ "$zeros" != "0" ]
while [ "$loopvar" -le "$loopnum" ]
do
    timestart=$(date +%s)

    while [ $timenow -le $(($timestart + $timestep)) ]
    do
        sleep 1
        timenow=$(date +%s)
    done    

    ./init_ct_test.sh $timenow
    echo $(date)

    #Check SNAP addresses
    addr_in=$( $path/regread.sh ct sys_snap_in_addr | awk '{print $3}')
    echo "addr_in: $addr_in"
    addr_mid=$( $path/regread.sh ct sys_snap_mid_addr | awk '{print $3}')
    echo "addr_mid: $addr_mid"
    addr_out=$( $path/regread.sh ct sys_snap_out_addr | awk '{print $3}')
    echo "addr_out: $addr_out"

    sync0=$( $path/regread.sh ct sys_dram_ctrl_sync_check0 | awk '{print $2 $3}')
    echo "sync0: $sync0 $((0x$sync0-0x$sync0o))"
    sync1=$( $path/regread.sh ct sys_dram_ctrl_sync_check1 | awk '{print $2 $3}')
    echo "sync1: $sync1 $((0x$sync1-0x$sync1o))"
    sync2=$( $path/regread.sh ct sys_dram_ctrl_sync_check2| awk '{print $2 $3}')
    echo "sync2: $sync2 $((0x$sync2-0x$sync2o))"
    sync3=$( $path/regread.sh ct sys_dram_ctrl_sync_check3| awk '{print $2 $3}')
    echo "sync3: $sync3 $((0x$sync3-0x$sync3o))"
    sync4=$( $path/regread.sh ct sys_dram_ctrl_sync_check4| awk '{print $2 $3}')
    echo "sync4: $sync4 $((0x$sync4-0x$sync4o))"

    echo $timenow $(date) $timestep $addr_in $addr_mid $addr_out $((0x$sync0-0x$sync0o)) $((0x$sync1-0x$sync1o)) $((0x$sync2-0x$sync2o)) $((0x$sync3-0x$sync3o)) $((0x$sync4-0x$sync4o)) >> "log_$logname.txt"

    if [[ "$addr_in" == "0000" || "$addr_mid" == "0000" || "$addr_out" == "0000" ]]
    then
#    zeros=0
        echo 0 > /proc/$ctPN/hw/ioreg_mode
        echo 1 > /proc/$ctPN/hw/ioreg/sys_dram_ctrl_dimm_reset
        sleep 1
        echo 0 > /proc/$ctPN/hw/ioreg/sys_dram_ctrl_dimm_reset
        echo 1 > /proc/$ctPN/hw/ioreg_mode
        echo "DIMM reset."
    fi

    $path/regread.sh ct sys_sync_in_test_bram_sync > "synctest_$logname.txt"
    $path/regread.sh ct sys_sync_out_test_bram_sync >> "synctest_$logname.txt" 

    loopvar=$(($loopvar + 1))
    echo $loopvar
done

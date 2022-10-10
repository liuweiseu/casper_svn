ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')

if [ -n "$ctPN" ]
then
#    echo "*****CT*****"
    echo 0 > /proc/$ctPN/hw/ioreg_mode

#SNAP CT out
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_in_we
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_in_ctrl
    sleep 4
    echo 0 > /proc/$ctPN/hw/ioreg/sys_snap_in_ctrl

    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_out_we
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_out_ctrl
    sleep 4
    echo 0 > /proc/$ctPN/hw/ioreg/sys_snap_out_ctrl
#    echo "ct_out_addr: $(cat /proc/$ctPN/hw/ioreg/sys_snap_out_addr)"

    echo 1 > /proc/$ctPN/hw/ioreg_mode

    ./read_bram11 $ctPN sys_snap_out c2c > ct_out_${1}.txt
#    echo "Reading SNAP_CT_OUT"
#    echo $header
#    awk 'NR>=0&&NR<=16' ct_out_${1}.txt

else
    echo "No CT model running."
fi

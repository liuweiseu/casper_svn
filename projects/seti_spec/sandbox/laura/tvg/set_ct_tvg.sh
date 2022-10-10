#! /bin/bash
#ARG 1,2,3 = length, skip stall

procPN=$(ps aux | grep bof | grep ct | awk '{print $2}')
length=${1}
skip=${2}
stall=${3}

ctrl_val=$((length*2**20 + stall*2**10 + skip))

echo 0 > /proc/$procPN/hw/ioreg_mode

echo $ctrl_val > /proc/$procPN/hw/ioreg/sys_tvg_in_tvg_ctrl
echo $ctrl_val
echo 1 > /proc/$procPN/hw/ioreg_mode

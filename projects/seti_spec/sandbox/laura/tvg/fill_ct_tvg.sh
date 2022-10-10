#!/bin/bash
#ARG 1 = scale
procPN=$(ps aux | grep bof | grep ct | awk '{print $2}')

rtvgname=sys_tvg_in_ram_re
itvgname=sys_tvg_in_ram_im
exepath=/home/laura/seti_spec/src/tvg/fill_tvg
filename=/home/laura/seti_spec/src/tvg/tvg_step_p1.dat

#Fill real TVG BRAM
$exepath $procPN 0 510 $rtvgname $filename ${1}
sleep 1

#Fill imag TVG BRAM
$exepath $procPN 0 510 $itvgname $filename ${1}

#Set MUX registers

echo 0 > /proc/$procPN/hw/ioreg_mode
sleep 1
echo 268435455 > /proc/$procPN/hw/ioreg/sys_sync_period
sleep 1
echo 1 > /proc/$procPN/hw/ioreg/sys_sync_in_sel
sleep 1
echo 1 > /proc/$procPN/hw/ioreg/sys_data_in_sel

echo 1 > /proc/$procPN/hw/ioreg_mode


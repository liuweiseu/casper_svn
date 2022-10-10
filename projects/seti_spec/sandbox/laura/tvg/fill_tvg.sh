#!/bin/bash
#ARG 1 = scale
procPN=$(ps aux | grep bof | grep pfb | awk '{print $2}')

rtvgname=data_mux_in_tvg_ram_re
itvgname=data_mux_in_tvg_ram_im

#Fill real TVG BRAM
/home/obs/seti_spec/src/tvg/fill_tvg $procPN 0 511 $rtvgname tvg_cos.dat ${1}
sleep 1

#Fill imag TVG BRAM
/home/obs/seti_spec/src/tvg/fill_tvg $procPN 0 511 $itvgname tvg_sin.dat ${1}

#Set MUX registers

echo 0 > /proc/$procPN/hw/ioreg_mode
sleep 1
echo 268435455 > /proc/$procPN/hw/ioreg/sync_in_mux_sync_period
sleep 1
echo 1 > /proc/$procPN/hw/ioreg/sync_in_mux_sync_in_sel
sleep 1
echo 1 > /proc/$procPN/hw/ioreg/data_mux_in_data_sel

echo 1 > /proc/$procPN/hw/ioreg_mode

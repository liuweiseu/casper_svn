#!/bin/bash
#arg 1 = bof type (pfb, ct, fft, thresh)
#arg 2 = snap name
#arg 3 = we reg name

procnum=$(ps aux | grep bof | grep ${1} | awk '{print $2}')

echo 0 > /proc/$procnum/hw/ioreg_mode
echo 1 > /proc/$procnum/hw/ioreg/${3}
echo 1 > /proc/$procnum/hw/ioreg/${2}_ctrl

sleep 4

echo 0 > /proc/$procnum/hw/ioreg/${2}_ctrl
echo 1 > /proc/$procnum/hw/ioreg_mode

echo "snap reset"

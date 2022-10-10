#!/bin/bash
#arg 1 = bof type (pfb, ct, fft, thresh)
#arg 2 = register name
#arg 3 = value to write to reg

numarg=$(echo $#)

if [ "$numarg" = "3" ]
then
    procnum=$(ps aux | grep bof | grep ${1} | awk '{print $2}')

    echo 0 > /proc/$procnum/hw/ioreg_mode
    echo ${3} > /proc/$procnum/hw/ioreg/${2}
    echo 1 > /proc/$procnum/hw/ioreg_mode
else
    echo "Usage: ./regwrite.sh <proc name string> <register or bram> <value>"
fi

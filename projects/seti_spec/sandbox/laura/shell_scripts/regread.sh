#!/bin/bash
#arg 1 = bof type (pfb, ct, fft, thresh)
#arg 2 = register name
numarg=$(echo $#)

if [ "$numarg" = "2" ]
then
    procnum=$(ps aux | grep bof | grep ${1} | awk '{print $2}')

    od -x /proc/$procnum/hw/ioreg/${2}
else
    echo "Usage: ./regread.sh <process name string> <register or bram name>"
fi

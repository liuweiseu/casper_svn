#!/bin/bash
#prints /proc directory for a bof file given keyword in name (pfb, ct, etc)
#if no argument given, simply lists all processes with "bof" string
numarg=$(echo $#)

if [ "$numarg" == "1" ]
then
    procnum=$(ps aux | grep bof | grep ${1} | awk '{print $2}')

    echo $procnum
    ls /proc/$procnum/hw/ioreg
else
    ps aux | grep bof
fi

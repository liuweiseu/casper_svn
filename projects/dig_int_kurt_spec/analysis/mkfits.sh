#!/bin/bash
newfits -i $1_ypower.bin -o 8 -s 1024 $2 $1_ypower.fits
newfits -i $1_xpower.bin -o 8 -s 1024 $2 $1_xpower.fits
newfits -i $1_ypowersq.bin -o -16 -s 1024 $2 $1_ypowersq.fits
newfits -i $1_xpowersq.bin -o -16 -s 1024 $2 $1_xpowersq.fits


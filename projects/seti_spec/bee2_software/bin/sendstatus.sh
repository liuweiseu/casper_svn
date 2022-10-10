#!/bin/sh

while [ 1 -eq 1 ]
do

    #Get 3 process IDs#
    pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')
    fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')
    thrproc=$(ps aux | grep bof | grep thr | awk '{print $2}')

    echo "Updating..."
    echo "BEE2 STATUS: `date`" > /tmp/bee2status
    echo "PFB SHIFT: `read_reg $pfbproc fft_shift`" >> /tmp/bee2status
    echo "FFT SHIFT: `read_reg $fftproc fft_shift`" >> /tmp/bee2status
    echo "THRESH LIMIT: `read_reg $thrproc thr_comp1_thr_lim`" >> /tmp/bee2status
    echo "THRESH SCALE: `read_reg $thrproc thr_scale_p1_scale`" >> /tmp/bee2status
    echo "TENGE PORT: `read_reg $thrproc rec_reg_10GbE_destport0`" >> /tmp/bee2status
    echo "TENGE IP: `read_reg $thrproc rec_reg_ip`" >> /tmp/bee2status

    nc -u -q 1 192.65.176.148 31337 < /tmp/bee2status
    sleep 8

done


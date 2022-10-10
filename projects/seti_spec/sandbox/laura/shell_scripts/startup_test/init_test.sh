#!/bin/bash

pfbPN=$(ps aux | grep bof | grep pfb | awk '{print $2}')
ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')
fftPN=$(ps aux | grep bof | grep fft | awk '{print $2}')
thrPN=$(ps aux | grep bof | grep t_t | awk '{print $2}')

if [ -n "$pfbPN" ]
then
    echo "*****PFB*****"

    echo 0 > /proc/$pfbPN/hw/ioreg_mode
    #check XAUI link status
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_linkdown)
    echo "rx_linkdown = $regval"
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_empty)
    echo "rx_empty = $regval"
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_valid)
    echo "rx_valid = $regval"
    echo "CORRECT: rx_linkdown = rx_empty = 0; rx_valid = 1"

    #Snap PFB SNAP blocks
    echo 1 > /proc/$pfbPN/hw/ioreg/snap_xaui_we
    sleep 2
    ./read_bram11 $pfbPN snap_xaui c2c > pfb_xaui.txt
    echo "***snap_xaui_lsb and snap_xaui_msb***"
    awk 'NR>=0&&NR<=16' pfb_xaui.txt

    echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_we
    sleep 2
    ./read_bram12 $pfbPN snap_fft c2c > pfb_fft.txt
    echo "***snap_fft_lsb and snap_fft_msb***"
    awk 'NR>=0&&NR<=16' pfb_fft.txt

    echo 1 > /proc/$pfbPN/hw/ioreg_mode
else
    echo "No PFB model running."
fi

if [ -n "$ctPN" ]
then
    echo "*****CT*****"
    echo 0 > /proc/$ctPN/hw/ioreg_mode

    #Snap CT SNAP blocks
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_in_we
    sleep 2
    ./read_bram11 $ctPN sys_snap_in c2c > ct_in.txt
    echo "***sys_snap_in_ct_lsb and sys_snap_in_ct_msb***"
    awk 'NR>=0&&NR<=16' ct_in.txt

    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_out_we
    sleep 2
    ./read_bram11 $ctPN sys_snap_out c2c > ct_out.txt
    echo "***sys_snap_out_ct_lsb and sys_snap_out_ct_msb***"
    awk 'NR>=0&&NR<=16' ct_out.txt

    echo 1 > /proc/$ctPN/hw/ioreg_mode
else
    echo "No CT model running."
fi

if [ -n "$fftPN" ]
then
    #Snap FFT SNAP blocks
    echo "*****FFT*****"

    echo 0 > /proc/$fftPN/hw/ioreg_mode

    echo 1 > /proc/$fftPN/hw/ioreg/snap_fft_we
    sleep 2
    ./read_bram11 $fftPN snap_fft c2c > fft_in.txt
    echo "***snap_fft_lsb and snap_fft_msb***"
    awk 'NR>=0&&NR<=16' fft_in.txt

    echo 1 > /proc/$fftPN/hw/ioreg_mode
else
    echo "No FFT model running."
fi

if [ -n "$thrPN" ]
then

    #Snap THRESHOLDER SNAP blocks
    echo "*****Thresholder*****"

    echo 0 > /proc/$thrPN/hw/ioreg_mode

    echo 1 > /proc/$thrPN/hw/ioreg/ct_in_mux_snap64_ct_we
    sleep 2
    ./read_bram11 $thrPN ct_in_mux_snap64_ct c2c > thr_ct_in.txt
    echo "***ct_in_mux_snap64_lsb and msb***"
    awk 'NR>=0&&NR<=16' thr_ct_in.txt


    echo 1 > /proc/$thrPN/hw/ioreg/fft_in_mux_snap64_fft_we
    sleep 2
    ./read_bram11 $thrPN fft_in_mux_snap64_fft c2c > thr_fft_in.txt
    echo "***fft_in_mux_snap64_lsb and msb***"
    awk 'NR>=0&&NR<=16' thr_fft_in.txt

    echo 1 > /proc/$thrPN/hw/ioreg_mode
else
    echo "No Thresholder model running."
fi

#!/bin/bash
# Script checks status registers and (re)snaps SNAP blocks
# Output of SNAP blocks saved to a text file and
# 16 lines are printed to the screen
# "get_bram11" and "get_bram12" from "read_all_bram.c"

pfbPN=$(ps aux | grep bof | grep pfb | awk '{print $2}')
ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')
fftPN=$(ps aux | grep bof | grep fft | awk '{print $2}')
thrPN=$(ps aux | grep bof | grep thr | awk '{print $2}')

header="CH ----- unsignLSB --- unsignMSB --- signLSB - signMSB - ReFFT -- ImFFT"

if [ -n "$pfbPN" ]
then
    echo "*****PFB*****"

    echo 0 > /proc/$pfbPN/hw/ioreg_mode
    
    #check XAUI link status
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_linkdown)
    echo "*****PFB*****"
    echo "rx_linkdown = $regval"
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_empty)
    echo "rx_empty = $regval"
    regval=$(cat /proc/$pfbPN/hw/ioreg/rx_valid)
    echo "rx_valid = $regval"
    echo "CORRECT: rx_linkdown = 0;  rx_empty = 0; rx_valid = 1"
    regval=$(cat /proc/$pfbPN/hw/ioreg/fft_shift)
    echo "fft_shift: $regval"
    sleep 1

    #Snap PFB SNAP blocks
    echo 1 > /proc/$pfbPN/hw/ioreg/snap_xaui_we
    echo 1 > /proc/$pfbPN/hw/ioreg/snap_xaui_ctrl
    echo 0 > /proc/$pfbPN/hw/ioreg/snap_xaui_ctrl
#    echo 1 > /proc/$pfbPN/hw/ioreg/snap_xaui_ctrl
    sleep 3
    echo 0 > /proc/$pfbPN/hw/ioreg/snap_xaui_we
#    echo 0 > /proc/$pfbPN/hw/ioreg/snap_xaui_ctrl


    echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_we
    echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl
    echo 0 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl
#    echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl
    sleep 2
    echo 0 > /proc/$pfbPN/hw/ioreg/snap_fft_we
#    echo 0 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl
    echo 1 > /proc/$pfbPN/hw/ioreg_mode

    echo "Reading SNAP_PFB_XAUI"
    ./read_bram11 $pfbPN snap_xaui c2c fft > pfb_xaui.txt
    echo $header
    awk 'NR>=0&&NR<=16' pfb_xaui.txt

    echo "Reading SNAP_PFB_FFT"
    ./read_bram12 $pfbPN snap_fft c2c > pfb_fft.txt
    echo $header
    awk 'NR>=0&&NR<=16' pfb_fft.txt

else
    echo "No PFB model running."
fi

if [ -n "$ctPN" ]
then
    echo "*****CT*****"
    echo 0 > /proc/$ctPN/hw/ioreg_mode

    #Snap CT SNAP blocks
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_in_we
    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_in_ctrl
    sleep 2
    echo 0 > /proc/$ctPN/hw/ioreg/sys_snap_in_ctrl

    echo 1 > /proc/$ctPN/hw/ioreg/sys_snap_out_we
    echo 2 > /proc/$ctPN/hw/ioreg/sys_snap_out_ctrl
    sleep 2
    echo 0 > /proc/$ctPN/hw/ioreg/sys_snap_out_ctrl

    echo 1 > /proc/$ctPN/hw/ioreg_mode

    ./read_bram11 $ctPN sys_snap_in c2c > ct_in.txt
    echo "Reading SNAP_CT_IN"
    echo $header
    awk 'NR>=0&&NR<=16' ct_in.txt

    ./read_bram11 $ctPN sys_snap_out c2c > ct_out.txt
    echo "Reading SNAP_CT_OUT"
    echo $header
    awk 'NR>=0&&NR<=16' ct_out.txt

else
    echo "No CT model running."
fi

if [ -n "$fftPN" ]
then
    #Snap FFT SNAP blocks
    echo "*****FFT*****"

    echo 0 > /proc/$fftPN/hw/ioreg_mode

    echo 1 > /proc/$fftPN/hw/ioreg/snap_fft_we
    echo 1 > /proc/$fftPN/hw/ioreg/snap_fft_ctrl
    sleep 2
    echo 0 > /proc/$fftPN/hw/ioreg/snap_fft_ctrl
    read_bram11 $fftPN snap_fft c2c > fft_in.txt
    echo "***snap_fft_lsb and snap_fft_msb***"
    echo $header
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
    echo 1 > /proc/$thrPN/hw/ioreg/ct_in_mux_snap64_ct_ctrl
    sleep 2
    echo 1 > /proc/$thrPN/hw/ioreg/ct_in_mux_snap64_ct_ctrl
    read_bram11 $thrPN ct_in_mux_snap64_ct c2c > thr_ct_in.txt
    echo "***ct_in_mux_snap64_lsb and msb***"
    echo $header
    awk 'NR>=0&&NR<=16' thr_ct_in.txt


    echo 1 > /proc/$thrPN/hw/ioreg/fft_in_mux_snap64_fft_we
    echo 1 > /proc/$thrPN/hw/ioreg/fft_in_mux_snap64_fft_ctrl
    sleep 2
    echo 0 > /proc/$thrPN/hw/ioreg/fft_in_mux_snap64_fft_ctrl
    ./read_bram11 $thrPN fft_in_mux_snap64_fft c2c > thr_fft_in.txt
    echo "***fft_in_mux_snap64_lsb and msb***"
    echo $header
    awk 'NR>=0&&NR<=16' thr_fft_in.txt

    echo 1 > /proc/$thrPN/hw/ioreg/snap_en
    sleep 1
    echo 1 > /proc/$thrPN/hw/ioreg/rec_snap_we
    sleep 1
    echo "Thresholder SNAP blocks snapped."

    echo 1 > /proc/$thrPN/hw/ioreg_mode
else
    echo "No Thresholder model running."
fi

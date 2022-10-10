#Sets the interconnect mode between the FPGAs on the BEE2
#GPIO 0
#XAUI 1
#!/bin/bash

pfbPN=$(ps aux | grep bof | grep pfb | awk '{print $2}')
ctPN=$(ps aux | grep bof | grep ct | awk '{print $2}')
fftPN=$(ps aux | grep bof | grep fft | awk '{print $2}')
thrPN=$(ps aux | grep bof | grep t_t | awk '{print $2}')
int_sel=$(echo ${1})

if [ "$int_sel" == "1" ];
then
    echo "Interconnect method set to XAUI"
elif [ "$int_sel" == "0" ];
then
    echo "Interconnect method set to GPIO"
else
    echo "Please select gpio (0) or XAUI (1)"
fi

### SET PFB ###

echo 1 > /proc/$pfbPN/ioreg_mode
echo $int_sel > /proc/$pfbPN/ioreg/gpio0_xaui1
echo 0 > /proc/$pfbPN/ioreg_mode

### SET CT ###

echo 1 > /proc/$ctPN/ioreg_mode
echo $int_sel > /proc/$ctPN/ioreg/gpio0_xaui1_in
echo $int_sel > /proc/$ctPN/ioreg/gpio0_xaui1_out
echo 0 > /proc/$ctPN/ioreg_mode

### SET FFT ### 

echo 1 > /proc/$fftPN/ioreg_mode
echo $int_sel > /proc/$fftPN/ioreg/gpio0_xaui1_in
echo $int_sel > /proc/$fftPN/ioreg/gpio0_xaui1_out
echo 0 > /proc/$fftPN/ioreg_mode

### SET THR ###

echo 1 > /proc/$thrPN/ioreg_mode
echo $int_sel > /proc/$thrPN/ioreg/gpio0_xaui1
echo 0 > /proc/$thrPN/ioreg_mode

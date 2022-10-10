pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')
fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')
thrproc=$(ps aux | grep bof | grep t_t | awk '{print $2}')

echo 0 > /proc/$thrproc/hw/ioreg_mode
echo 10 > /proc/$thrproc/hw/ioreg/thr_comp1_thr_lim
echo 1000 > /proc/$thrproc/hw/ioreg/thr_scale_p1_scale
echo 1 > /proc/$thrproc/hw/ioreg_mode

echo 0 > /proc/$fftproc/hw/ioreg_mode
echo 32767 > /proc/$fftproc/hw/ioreg/fft_shift
echo 1 > /proc/$fftproc/hw/ioreg_mode

echo 0 > /proc/$pfbproc/hw/ioreg_mode
echo 2147483647 > /proc/$pfbproc/hw/ioreg/fft_shift
#echo 268435455 > /proc/$pfbproc/hw/ioreg/sync_in_mux_sync_period
#echo 1 > /proc/$pfbproc/hw/ioreg/sync_in_mux_sync_in_sel
#echo 1 > /proc/$pfbproc/hw/ioreg/data_mux_in_data_sel
#echo "Sets up PFB TVG. NOT REAL DATA"
echo 1 > /proc/$pfbproc/hw/ioreg_mode



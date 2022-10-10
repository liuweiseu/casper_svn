pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')
thrproc=$(ps aux | grep bof | grep thr | awk '{print $2}')
fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')

#====THRESHOLDER=====#

#set the max hits for each pfb bin 
write_reg $thrproc thr_comp1_thr_lim 5

#set the scaling factor to x/2^5
write_reg $thrproc thr_scale_p1_scale 20

#configure 10GbE
cp 10gbe.conf /proc/$thrproc/hw/ioreg/rec_ten_GbE

write_reg $thrproc rec_reg_10GbE_destport0 33001
write_reg $thrproc rec_reg_ip 167772161

#write_reg  $thrproc rec_tge_reset 1
#write_reg  $thrproc rec_tge_resethh 1
#write_reg  $thrproc rec_tge_reset 0
#write_reg  $thrproc rec_tge_resethh 0

#=====PFB=====#

write_reg $pfbproc fft_shift 268435455 

#=====FFT=====#

write_reg $fftproc fft_shift 32767 


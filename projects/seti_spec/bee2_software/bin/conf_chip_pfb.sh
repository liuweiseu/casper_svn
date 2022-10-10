pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')

#=====PFB=====#
write_reg $pfbproc fft_shift 268435455 

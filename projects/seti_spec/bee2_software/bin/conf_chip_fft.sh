fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')

#=====FFT=====#
write_reg $fftproc fft_shift 28398 


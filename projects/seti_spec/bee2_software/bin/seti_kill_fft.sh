fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')

if [ $fftproc ]
    then
	kill -9 $fftproc
fi

pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')
thrproc=$(ps aux | grep bof | grep thr | awk '{print $2}')
ctproc=$(ps aux | grep bof | grep ct | awk '{print $2}')
fftproc=$(ps aux | grep bof | grep fft | awk '{print $2}')

if [ $pfbproc ]
    then
	kill -9 $pfbproc
fi

if [ $fftproc ]
    then
	kill -9 $fftproc
fi

if [ $ctproc ]
    then
	kill -9 $ctproc
fi

if [ $thrproc ]
    then
	kill -9 $thrproc
fi

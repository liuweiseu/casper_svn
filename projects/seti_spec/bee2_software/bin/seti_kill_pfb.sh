pfbproc=$(ps aux | grep bof | grep pfb | awk '{print $2}')

if [ $pfbproc ]
    then
	kill -9 $pfbproc
fi

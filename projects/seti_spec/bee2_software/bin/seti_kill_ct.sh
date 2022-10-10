ctproc=$(ps aux | grep bof | grep ct | awk '{print $2}')

if [ $ctproc ]
    then
	kill -9 $ctproc
fi

thrproc=$(ps aux | grep bof | grep thr | awk '{print $2}')

if [ $thrproc ]
    then
	kill -9 $thrproc
fi

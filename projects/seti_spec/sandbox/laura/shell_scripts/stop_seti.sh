procnum=$(ps aux | grep bof | grep t_t | awk '{print $2}')

kill -9 $procnum
sleep 1

procnum=$(ps aux | grep bof | grep fft | awk '{print $2}')

kill -9 $procnum
sleep 1

procnum=$(ps aux | grep bof | grep ct | awk '{print $2}')

kill -9 $procnum
sleep 1

procnum=$(ps aux | grep bof | grep pfb | awk '{print $2}')

kill -9 $procnum


(cat setup_data  | nc -u -p 30718 192.168.4.240 30718 > /dev/null & sleep 1 && exit) ; pkill -9 nc
(cat reset_data  | nc -u -p 30718 192.168.4.20 30718 > /dev/null & sleep 1 && exit)  ; pkill -9 nc

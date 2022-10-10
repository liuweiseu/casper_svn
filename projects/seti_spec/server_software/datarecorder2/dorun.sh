#!/bin/sh

ibob_ip="192.168.2.6"
bee2_name="beecourageous"

echo "copying dr2_config_short into dr2_config"
cp dr2_config_short dr2_config

echo "validating sudo on pattern"
sudo -v

rm nohup.out

# kill previous run with SIGINT
# the software is designed to handle and close cleanly
echo "killing any previous setispec_dr runs"
sudo killall -2 -q setispec_dr
sleep 30
# if we could not close cleanly force kill
sudo killall -9 -q setispec_dr

echo "killing previous ssh instances to run sendstatus on ${bee2_name}"
sshsendtatus=$(ps aux | grep ${bee2_name} | grep sendstatus | awk '{print $2}')
sudo kill -9 $sshsendtatus

echo "initalizing the iBOB"
echo "regw ddc_en 1" | nc ${ibob_ip} 7 -u -w 1
./beamswitch 7beam2pol 0 1
echo "regw blank_radar 0" | nc ${ibob_ip} 7 -u -w 1

# reboot the bee
echo "validating sudo on ${bee2_name}"
ssh ${bee2_name} -t "sudo -v"
echo "rebooting the bee2"
ssh ${bee2_name} -t "sudo ./start_seti.sh"
echo "starting sendstatus"
nohup ssh ${bee2_name} -t "sudo /home/tfiliba/bin/sendstatus.sh" &
sleep 20

echo "starting diskbuf cleaner"
sudo ./collect_diskbufs.bash > /dev/null &

echo "starting new run"
sudo ./setispec_dr &> output

sudo killall -9 collect_diskbufs.bash

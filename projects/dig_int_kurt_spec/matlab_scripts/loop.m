m = [0,0,0,0];

ibobexec('192.168.1.151', 'regwrite snap_ddc/ctrl 0')
pause(0.4)
ibobexec('192.168.1.151', 'regwrite snap_ddc/ctrl 5')
pause(0.4)
ibobexec('192.168.1.151', 'regwrite snap_ddc/ctrl 3')

pause(0.4)

x = adcsnap('192.168.1.151', 'bramdump snap_ddc/bram');

m = cat(1,m,x);


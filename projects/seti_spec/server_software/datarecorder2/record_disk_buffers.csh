#! /bin/tcsh -f

set disk_buff_count = 0
date
    
while(1)
    set EOT = `mt -f /dev/nst0 sta | grep -c EOT`
    if ($EOT >= 1) then
        set dr2_pid = `ps -ef | grep "dr2 -config" | grep -v grep | awk '{print $2}'`    
        kill $dr2_pid
        #echo recorded $disk_buff_count disk buffers
        date
        exit 0
    endif

    foreach i (`ls -1tr /ramdisk/*.done`)
	    echo recording $i
        # no rewind, ultra density, compressed ?
        # dd bs=1048576 if=$i of=/dev/nst0
        dd bs=1052672 if=$i of=/dev/nst0
	    \rm $i
        @ disk_buff_count += 1
        echo recorded $disk_buff_count disk buffers
    end

	sleep 3
end

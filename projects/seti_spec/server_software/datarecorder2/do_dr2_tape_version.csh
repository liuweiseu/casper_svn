#! /bin/tcsh -b

setenv LD_LIBRARY_PATH /opt/EDTpcd
setenv TAPE /dev/nst0
umask  000

set send_email                  = 0
set send_audio                  = 0
set operator_request_interval   = 300  # seconds
set alert_email_recipients      = "operators@naic.edu operator@naic.edu jeffc@ssl.berkeley.edu serendip@naic.edu"
set audio_file                  = files/message.au
set audio_access                = src/audio_dev_access
set notification_host           = "teton"  

set record_tape                 = 0
set disabled                    = 0
set append                      = 0
set overwrite                   = 0
set nogo                        = 0
set keep_online                 = 0
set dr2_config                  = /home/seti/files/dr2_config
set tape_file_name              = /home/seti/files/tapenamefile
set delete_disk_buffers_exec    = /home/seti/bin/delete_disk_buffers.csh
set record_disk_buffers_exec    = /home/seti/bin/record_disk_buffers.csh
set record_disk_buffers_log     = /tmp/record_disk_buffers_log
set dr2_exec                    = /home/seti/bin/dr2
set this_name                   = ""

set check_pid = `ps -ef | grep $dr2_exec | grep -v grep | awk '{print $2}'`
if (${#check_pid} > 0) then
    echo $dr2_exec is already running!
    exit 1
endif
#echo initializing...
#sleep 5
#set check_pid = `ps -ef | grep $0 | grep -v grep | awk '{print $2}'`
#if (${#check_pid} > 1) then
#    echo $0 is already running!
#    # ps -ef | grep $0 | grep -v grep 
#    exit 1
#endif


\rm /ramdisk/diskbuf_* >& /dev/null

while (! $disabled)

    if ($record_tape) then
    	if (x$this_name == x) then
        	set this_name = `tail -1 $tape_file_name | awk '{print $1}'` 
    	endif

    	set fn = `mt sta | grep "File number" | awk '{print $2}' | awk -F\= '{print $2}' | sed -s s/,/\/`
    	while ($fn == -1)
        
        	# console message
        	echo ""
        	echo ""
        	echo ""
        	date
        	echo Berkeley SETI@Home Data Recorder                       | tee /tmp/operator_mail_msg
        	echo please insert a new tape                               | tee -a /tmp/operator_mail_msg
        	echo if an old tape is ejected, please label it $this_name  | tee -a /tmp/operator_mail_msg
        	echo ""                                                     >> /tmp/operator_mail_msg
        	echo "thank you!"                                           >> /tmp/operator_mail_msg

        	# email operator
        	if ($send_email) then
            		mail -s "SETI@Home needs a new tape" $alert_email_recipients < /tmp/operator_mail_msg >& /dev/null 
        	endif

        	# audio request
        	if ($send_audio) then
            		rsh $notification_host -l serendip $audio_access $audio_file
        	endif

        	echo sleeping...
        	sleep $operator_request_interval

        	set fn = `mt sta | grep "File number" | awk '{print $2}' | awk -F\= '{print $2}' | sed -s s/,/\/`

    	end

    	if($fn == 0) then

        	# tape is positioned at the bottom

        	echo testing for data on tape...
        	mt rewind
        	# dd bs=1048576 count=1 if=$TAPE of=/tmp/tape_snippet >& /dev/null
        	dd bs=1052672 count=1 if=$TAPE of=/tmp/tape_snippet >& /dev/null
        	mt rewind
        	set tmpvar = `strings /tmp/tape_snippet | head -1 | grep ^HEADER`
        	if(x$tmpvar == x) then
            		echo tape has no data
            		set hasdata = 0
        	else 
            		echo tape has data
            		set hasdata = 1
        	endif

        	if ($hasdata && $append) then
            		echo moving tape to EOD
            		mt eod
        	endif

    	else

        	# tape is positioned at EOD (?)

        	if($overwrite) then
            		echo rewinding tape
            		mt rewind
        	else
            		if(! $append) then
                		echo both overwriting and appending are disallowed - skipping
                		set nogo = 1
            		endif
        	endif

    	endif

    endif  # if($record_tape)

    if (! $nogo) then

        set LASTFILE = `tail -1 $tape_file_name |& grep -v tail | awk '{print $1}'`
        if($append) then
            set this_name = $LASTFILE
        else
            set LASTDMY = `echo $LASTFILE | cut -c-6`
            set LASTFIRST = `echo $LASTFILE | cut -c7`
            set LASTSECOND = `echo $LASTFILE | cut -c8`

            set NOWDAY = `date '+%d'`
            set NOWMONTH = `date '+%m'`
            set NOWYEAR = `date '+%y'`
    
            switch ($NOWMONTH)
            case 01:
                set LETMONTH = ja
                breaksw
            case 02:
                set LETMONTH = fe
                breaksw
            case 03:
                set LETMONTH = mr
                breaksw
            case 04:
                set LETMONTH = ap
                breaksw
            case 05:
                set LETMONTH = my
                breaksw
            case 06:
                set LETMONTH = jn
                breaksw
            case 07:
                set LETMONTH = jl
                breaksw
            case 08:
                set LETMONTH = au
                breaksw
            case 09:
                set LETMONTH = se
                breaksw
            case 10:
                set LETMONTH = oc
                breaksw
            case 11:
                set LETMONTH = no
                breaksw
            case 12:
                set LETMONTH = dc
                breaksw
            endsw

            set NOWDMY = $NOWDAY$LETMONTH$NOWYEAR

            if ($NOWDMY == $LASTDMY) then # date the same, so aa->ab->ac...

                set NEWFIRST = $LASTFIRST

                if ($LASTSECOND != "z") then
                    set NEWSECOND = `echo $LASTSECOND | tr a-y b-z`
                else
                    set NEWSECOND = a
                    set NEWFIRST = `echo $LASTFIRST | tr a-y b-z`
                endif

                set this_name = $NOWDMY$NEWFIRST$NEWSECOND

            else                             # new day, so start with aa
                set this_name = $NOWDMY"aa"

            endif
    
            echo -n $this_name" " >> $tape_file_name
            date >> $tape_file_name

        endif

       set tape_pid = 0
       if ($record_tape) then
            set tape_pid = `ps -ef | grep "record_disk_buffers.csh" | grep -v grep | awk '{print $2}'`
       else
            set tape_pid = `ps -ef | grep "delete_disk_buffers.csh" | grep -v grep | awk '{print $2}'`
       endif
       while(${#tape_pid})
            kill ${tape_pid[1]}
            shift tape_pid
       end

       if ($record_tape) then
            $record_disk_buffers_exec |& grep -v "No match" >> $record_disk_buffers_log &
       else 
            $delete_disk_buffers_exec >& /dev/null &
       endif

       $dr2_exec -config $dr2_config -name $this_name

       set tape_pid = 0 
       if ($record_tape) then
            set tape_pid = `ps -ef | grep "record_disk_buffers.csh" | grep -v grep | awk '{print $2}'`
       else
            set tape_pid = `ps -ef | grep "delete_disk_buffers.csh" | grep -v grep | awk '{print $2}'`
       endif
       while(${#tape_pid})
            kill ${tape_pid[1]}
            shift tape_pid
       end

       # save off log for archiving
       set ext = `date '+%m_%d_%y_%H_%M_%S'`
       \mv /tmp/dr2_log /tmp/dr2_log.$ext

    endif

    if ($record_tape && ! $keep_online) then
       mt off 
    endif

end

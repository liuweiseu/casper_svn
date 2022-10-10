#! /bin/csh -f

while(1)
	ls /ramdisk/*.done
	rm /ramdisk/*.done
	sleep 3
end

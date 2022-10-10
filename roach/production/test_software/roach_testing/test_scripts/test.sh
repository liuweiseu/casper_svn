#/bin/bash
  COUNTER=0
  FOUND=0
  while [ $FOUND -eq 0 ]; do
    sleep 1
    if cat borph_load.log | grep -B 1 "Copy to Flash... done" | grep -q "cp.b 0x200000 0xfc200000 0x400000"; then
      echo "done." 
      FOUND=1
    elif [ $COUNTER -eq 60 ]; then
      echo "error: copy to flash not sucessfull...exiting."
      exit 1
    else  
      let COUNTER=COUNTER+1
    fi
  done


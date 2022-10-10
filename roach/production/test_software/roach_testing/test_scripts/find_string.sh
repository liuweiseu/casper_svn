#/bin/bash

if [ ! $1 ] ; then
  echo "usage: $0 file_to_search string timout_in_seconds"
  exit 1
fi

if [ ! "$2" ] ; then
  echo "usage: $0 file_to_search string timout_in_seconds"
  exit 1
fi

if [ ! $3 ] ; then
  echo "usage: $0 file_to_search string timout_in_seconds"
  exit 1
fi

FILE=$1
STRING="$2"
TIMEOUT=$3

  COUNTER=0
  while [ $COUNTER -lt $TIMEOUT ]; do
    sleep 1 
    cat $FILE | grep -q "$STRING"
    if [ $? -eq 0 ]; then
      exit 0
    else
      let COUNTER=COUNTER+1
    fi
  done
  exit 1


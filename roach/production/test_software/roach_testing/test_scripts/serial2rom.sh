#!/bin/bash
if [ ! $1 ]; then
  echo "error: usage $0 serialnumber prototype.ufc destination.ufc"
  exit 1
fi
if [ ! $2 ]; then
  echo "error: usage $0 serialnumber prototype.ufc destination.ufc"
  exit 1
fi
if [ ! $3 ]; then
  echo "error: usage $0 serialnumber prototype.ufc destination.ufc"
  exit 1
fi

PROTOTYPE=$2
DEST=$3


if ! echo $1 | grep -q '^\w\w\w\w\w\w$' ; then
  echo "invalid serial number"
  exit 1
fi

TEMP1=/tmp/deleteme1
TEMP2=/tmp/deleteme2

SERIAL=$1
cp $2 $TEMP1 > /dev/null
> $TEMP2

for ((i=0; i < 6;i=i+1)) ; do
  VAL=`echo -n ${SERIAL:i:1} | xxd | cut -f 2 -d ' '`
  cat $TEMP1 | sed -e 's#<Region name="R'$[120 + i]'"><content><static_data><fixed><value>..</value>#<Region name="R'$[120 + i]'"><content><static_data><fixed><value>'$VAL'</value>#g' > $TEMP2
  cp $TEMP2 $TEMP1
done
cp $TEMP1 $DEST
rm $TEMP1 $TEMP2

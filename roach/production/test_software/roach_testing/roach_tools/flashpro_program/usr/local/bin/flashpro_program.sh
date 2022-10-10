#!/bin/bash
TMPDIR=/tmp/

PROTOTYPE=/usr/local/lib/flashpro/prototype.pro

if ! [ $1 ] ; then
  echo "error: no STAPL file provided"
  exit 1
fi

STPL=$1

STPL_WINPATH=`cygpath -m $STPL | sed -e 's#/#\\\\\\\\#g'`

TMPDIR_WINPATH=`cygpath -m $TMPDIR | sed -e 's#/#\\\\\\\\#g'`

FLASHPRO_PROJ=${TMPDIR}flashpro.pro
FLASHPRO_PROJ_WINPATH=${TMPDIR_WINPATH}flashpro.pro

FLASHPRO_BIN=flashpro

cat $PROTOTYPE | sed -e "s#PROJFOLDER#${TMPDIR_WINPATH}#" \
                     -e "s#LOGFILE#${TMPDIR_WINPATH}flashpro.log#" \
                     -e "s#FILE#${STPL_WINPATH}#" > $FLASHPRO_PROJ

$FLASHPRO_BIN -run_project $FLASHPRO_PROJ_WINPATH

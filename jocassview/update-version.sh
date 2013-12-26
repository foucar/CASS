#!/bin/bash

OUTFILE=jocassview_version.h
INCLUDEGUARD=_JOCASSVIEW_VERSION

echo "Checking version..."

# Don't update the version if nothing needs to be compiled
numFiles=`make -n | wc -l`

if [ $numFiles -lt 6 ]; then
    echo "Nothing to update. Project is up to date!"
    exit
fi

echo "Updating version."

if [ -f $OUTFILE ]; then
  awk '{if ($2 ~ /MY_BUILD_NUMBER/) print $1 " " $2 " " ++$3; else print }' $OUTFILE > _$OUTFILE
  mv _$OUTFILE $OUTFILE
else
  echo "#ifndef $INCLUDEGUARD
#define $INCLUDEGUARD

#define MY_BUILD_NUMBER 1

#endif" > $OUTFILE
fi

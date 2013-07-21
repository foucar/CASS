#!/bin/bash

OUTFILE=cass_version.h
INCLUDEGUARD=_CASS_VERSION
unset GIT_DIR
unset GIT_WORK_TREE
VERSION=$(git describe --abbrev=4)

#echo "Checking version..."
#
## Don't update the version if nothing needs to be compiled
#numFiles=`make -n | wc -l`
#
#if [ $numFiles -lt 6 ]; then
#    echo "Nothing to update. Project is up to date!"
#    exit
#fi
#
#echo "Updating version."
#
#if [ -f $OUTFILE ]; then
#  awk '{if ($2 ~ /MY_BUILD_NUMBER/) print $1 " " $2 " " ++$3; else print }' $OUTFILE > _$OUTFILE
#  mv _$OUTFILE $OUTFILE
#else
#  echo "#ifndef $INCLUDEGUARD
##define $INCLUDEGUARD
#
##define MY_BUILD_NUMBER 1
#
##endif" > $OUTFILE
#fi

if [ -f ${OUTFILE} ] && [ $(cat ${OUTFILE} | grep ${VERSION} | wc -l) -eq 1 ]; then
  echo "${OUTFILE} does contain the latest information."
else
  echo "Generating file for VERSION: v${VERSION}"
  echo "#ifndef ${INCLUDEGUARD}
#define ${INCLUDEGUARD}

namespace cass
{
  std::string VERSION(\"v${VERSION}\");
}

#endif" > ${OUTFILE}
fi

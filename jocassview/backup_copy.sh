#!/bin/bash

installpath=$1
binname=$2

if [ ! -d $installpath/bin/jocassview_backup ]
then
  mkdir -p $installpath/bin/jocassview_backup
fi
ver=`cat jocassview_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp ../bin/$binname $installpath/bin/jocassview_backup/jocassview_backup_buildnr_${ver}_gitrev_`git describe --tags`_`date +%F_%I:%M`

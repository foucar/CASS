#!/bin/bash

installpath=$1

if [ ! -d $installpath/bin/jocassview_backup ]
then
  mkdir -p $installpath/bin/jocassview_backup
fi
ver=`cat jocassview_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp jocassview $installpath/bin/jocassview_backup/jocassview_backup_buildnr_${ver}_svnrev_`svnversion -n`_`date +%F_%I:%M`

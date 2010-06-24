#!/bin/bash

installpath=$1

#set -x
if [ ! -d $installpath/bin/cass_backup ]
then
  mkdir -p $installpath/bin/cass_backup
fi

ver=`cat cass_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp cass $installpath/bin/cass_backup/cass_backup_buildnr_${ver}_svnrev_`svnversion -n`_`date +%F_%I:%M`

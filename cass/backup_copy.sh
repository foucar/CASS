#!/bin/bash

installpath=$1
binname=$2

#set -x
if [ ! -d $installpath/bin/cass_backup ]
then
  mkdir -p $installpath/bin/cass_backup
fi

ver=`cat cass_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp ../bin/$binname $installpath/bin/cass_backup/cass_offline_backup_buildnr_${ver}_gitrev_`git describe --tags`_`date +%F_%I:%M`

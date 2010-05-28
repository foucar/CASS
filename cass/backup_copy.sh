mkdir cass_backup

ver=`cat cass_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp cass cass_backup/cass_backup_buildnr_${ver}_svnrev_`svnversion -n`_`date +%F_%k:%M`

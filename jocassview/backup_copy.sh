mkdir jocassview_backup

ver=`cat jocassview_version.h | grep MY_BUILD_NUMBER`
ver=${ver:24}
cp jocassview jocassview_backup/jocassview_backup_buildnr_${ver}_svnrev_`svnversion -n`_`date +%F_%k:%M`


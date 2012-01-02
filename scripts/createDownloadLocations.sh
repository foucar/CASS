#!/bin/bash

### set parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# tell where the download loacation is
DL=$HOME/public_html/Downloads

# tell the url of the download location
DLURL=http://www.mpi-hd.mpg.de/personalhomes/gitasg/Downloads



### create the zip and txt files

# remove the old downloads
rm -rf $DL/cass.*

# create a zip file with the contents of the master branch
cd $BASEDIR && env -i git archive master | gzip > $DL/cass.latest.tar.gz

# remove previous contents of the file containing the locations of the dowloads
echo "" > $BASEDIR/doc/TAGDownloads.txt

# go through all tags and create a zip file for them and put their download
# location into the text file
for tag in $(env -i git tag);
do
    echo "$DLURL/cass.$tag.tar.gz" >>  $BASEDIR/doc/TAGDownloads.txt
    env -i git archive $tag | gzip > $DL/cass.$tag.tar.gz
done


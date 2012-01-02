#!/bin/bash

### set some parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# the location where the html files should be copied to
PUBLIC_WWW=$HOME/public_html/cass/

# tell where the doxygen program is
DOXYGEN=/lfs/l3/asg/bin/doxygen



#### Create the doxygen documentation

# change to the right directory
cd $BASEDIR

# if we create the "official" documentation create the download locations first
# comment if not creating the docu for the official documentation
$BASEDIR/scripts/createDownloadLocations.sh $BASEDIR

# run doxygen from the docu directory and add the current version to the description
cd $BASEDIR/doc && (cat Doxyfile ; echo "PROJECT_NUMBER=`env -i git describe --tags`") | $DOXYGEN -

# now delete the existing webpage and copy the just created html files to the
# webpage and set the permissions correctly
rm -rf $PUBLIC_WWW && cp -r $BASEDIR/doc/doxygen/html/ $PUBLIC_WWW
find $PUBLIC_WWW -type f -print0 | xargs -0 chmod 640
find $PUBLIC_WWW -type d -print0 | xargs -0 chmod 750

#!/bin/bash

### set parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# tell where the download loacation is
DOWNLOAD=${HOME}/public_html/Downloads

# the location where the html files should be copied to
PUBLIC_WWW=${HOME}/public_html/cass/



### create the zip and txt files

# remove the old downloads
rm -rf ${DOWNLOAD}/cass.*

# create a zip file with the contents of the master branch
cd ${BASEDIR}

unset GIT_DIR
unset GIT_WORK_TREE
git archive master | gzip > ${DOWNLOAD}/cass.latest.tar.gz

# go through all tags and create a zip file for them and put their download
# location into the text file
for tag in $(env -i git tag)
do
  git archive ${tag} | gzip > ${DOWNLOAD}/cass.${tag}.tar.gz
done

# generate the documentation
cd ${BASEDIR}/doc && make doc && cd ..

### move the docu to the webpage location

# delete the existing webpage and copy the html files to the
# webpage and set the permissions correctly
rm -rf ${PUBLIC_WWW} && cp -r ${BASEDIR}/doc/doxygen/html/ ${PUBLIC_WWW}
find ${PUBLIC_WWW} -type f -exec chmod 640 {} \;
find ${PUBLIC_WWW} -type d -exec chmod 750 {} \;

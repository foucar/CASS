#!/bin/bash

### set parameters

# set the base directory of CASS (given by first commandline parameter)
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# tell where the download location is
DOWNLOAD=${HOME}/public_html/Downloads

# the location where the html files should be copied to
PUBLIC_WWW=${HOME}/public_html/cass/

# Path to the doxygen version
DOXYGEN=/lfs/l4/ullrch/foucar/cass/dependencies/doxygen/v1.8.10/bin/doxygen


### create the zip and txt files

# remove the old downloads
rm -rf ${DOWNLOAD}/cass.* || exit 1

# create a zip file with the contents of the master branch
cd ${BASEDIR} || exit 1

unset GIT_DIR
unset GIT_WORK_TREE
git archive master | gzip > ${DOWNLOAD}/cass.latest.tar.gz || exit 1

# go through all tags and create a zip file for them
for tag in $(env -i git tag)
do
  git archive ${tag} | gzip > ${DOWNLOAD}/cass.${tag}.tar.gz || exit 1
done

# generate the documentation (need to prepare the default config file first"
echo "CONFIG      *= DOCS"         >> ${BASEDIR}/cass_defaultconfig.pri || exit 1
echo "DOXYGEN_BIN  = "${DOXYGEN}"" >> ${BASEDIR}/cass_defaultconfig.pri || exit 1
cd ${BASEDIR}/doc && make doc && cd .. || exit 1

### move the docu to the webpage location

# delete the existing webpage and copy the html files to the
# webpage and set the permissions correctly
rm -rf ${PUBLIC_WWW} && cp -r ${BASEDIR}/doc/doxygen/html/ ${PUBLIC_WWW} || exit 1
find ${PUBLIC_WWW} -type f -exec chmod 640 {} \; || exit 1
find ${PUBLIC_WWW} -type d -exec chmod 750 {} \; || exit 1

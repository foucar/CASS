#!/bin/bash

### set the right environment variables and parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# the location where CASS should be installed to
INSTLOC=/lfs/l3/asg


# set the environment variables
export ROOTSYS=/lfs/l3/asg/root_v5.28.00c
QTDIR=/usr/local/Packages/qt-4.6.1
HDF5DIR=/lfs/l3/asg
GSOAPDIR=/lfs/l3/asg

PATH=$ROOTSYS/bin:$QTDIR/bin:$HDF5DIR/bin:$GSOAPDIR/bin:$PATH
CPLUS_INCLUDE_PATH=$QTDIR/include:$GSOAPDIR/include:$HDF5DIR/include:
LIBRARY_PATH=$GSOAPDIR/lib:$HDF5DIR/lib:$LIBRARY_PATH
LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH




### start build and install process

# ususally one now copies the cass_defaultconfig.pri file to 
# cass_myconfig.pri and then edits the cass_myconfig.pri file by hand. 
# Since this script should automatically install CASS we need to modify the 
# cass_defaultconfig.pri file from within this script. We add all options that
# we want to have enabled just at the end of the file.
echo "CONFIG += hdf5"               >> $BASEDIR/cass_defaultconfig.pri
echo "CONFIG += singleparticle_hit" >> $BASEDIR/cass_defaultconfig.pri
echo "CONFIG += cernroot"           >> $BASEDIR/cass_defaultconfig.pri
echo "CONFIG += JoCASSView"         >> $BASEDIR/cass_defaultconfig.pri
echo "CONFIG += LuCASSView"         >> $BASEDIR/cass_defaultconfig.pri

# make a clean build of the binaries and install them in the right location for
# the users to run the latest stable version of CASS
cd $BASEDIR && qmake INSTALLBASE=$INSTLOC && make && make install

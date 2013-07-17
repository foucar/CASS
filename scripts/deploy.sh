#!/bin/bash

### set the right environment variables and parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# the location where CASS should be installed to
INSTLOC=/lfs/l3/asg/cass/v$(env -i git describe --abbrev=4)


# set the environment variables
export ROOTSYS=/lfs/l3/asg/root_v5.32.00
export QTDIR=/lfs/l3/asg/qt/4.8.2
export HDF5DIR=/lfs/l3/asg
export GSOAPDIR=/lfs/l3/asg
export QWTDIR=/lfs/l3/asg
export VIGRADIR=/lfs/l3/asg

export PATH=$ROOTSYS/bin:$QTDIR/bin:$GSOAPDIR/bin:$PATH




### start build and install process

# ususally one now copies the cass_defaultconfig.pri file to 
# cass_myconfig.pri and then edits the cass_myconfig.pri file by hand. 
# Since this script should automatically install CASS we need to modify the 
# cass_defaultconfig.pri file from within this script. We add all options that
# we want to have enabled just at the end of the file.
echo "CONFIG += hdf5"                         >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   += "${HDF5DIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   += "${HDF5DIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR += "${HDF5DIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG += singleparticle_hit"          >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR += "${VIGRADIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG += cernroot"                     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   += "${ROOTSYS}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   += "${ROOTSYS}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR += "${ROOTSYS}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG += JoCASSView"                  >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   += "${QWTDIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   += "${QWTDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR += "${QWTDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG += LuCASSView"                  >> ${BASEDIR}/cass_defaultconfig.pri


#QMAKE_CXXFLAGS += -fopenmp
#QMAKE_LFLAGS   += -fopenmp
#DEFINES        += _GLIBCXX_PARALLEL   #enables openmp support for gcc stdlibc++




# make a clean build of the binaries and install them in the right location for
# the users to run the latest stable version of CASS
cd ${BASEDIR} && qmake PREFIX=${INSTLOC} && make && make install

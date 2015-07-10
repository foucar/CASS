#!/bin/bash

### set the right environment variables and parameters

# set the base directory of CASS (given by first commandline parameter
# or set by hand
BASEDIR=$1
#BASEDIR=/path/to/CASS/base/directory

# the location where CASS should be installed to
unset GIT_DIR
unset GIT_WORK_TREE
INSTLOC=/lfs/l4/ullrch/foucar/cass/v$(git describe --abbrev=4)


# set the environment variables
export ROOTSYS=/lfs/l4/ullrch/foucar/dependencies/root/v5.32.04
export QTDIR=/lfs/l4/ullrch/foucar/dependencies/qt/v4.8.4
export HDF5DIR=/lfs/l4/ullrch/foucar/dependencies/hdf5/v1.8.12
export GSOAPDIR=/lfs/l4/ullrch/foucar/dependencies/gsoap/v2.8.17
export QWTDIR=/lfs/l4/ullrch/foucar/cass/dependencies/qwt/v6.1.0
export VIGRADIR=/lfs/l4/ullrch/foucar/dependencies/vigra/v1.10.0
export DOXYDIR=/lfs/l4/ullrch/foucar/cass/dependencies/doxygen/v1.8.4
export FFTWDIR=/lfs/l4/ullrch/foucar/cass/dependencies/fftw/v3.3.3

export PATH=$QTDIR/bin:$PATH




### start build and install process

# usually one now copies the cass_defaultconfig.pri file to
# cass_myconfig.pri and then edits the cass_myconfig.pri file by hand.
# Since this script should automatically install CASS we need to modify the
# cass_defaultconfig.pri file from within this script. We add all options that
# we want to have enabled just at the end of the file.
echo "QMAKE_INCDIR   *= "${GSOAPDIR}"/include/"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   *= "${GSOAPDIR}"/lib"          >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR *= "${GSOAPDIR}"/lib"          >> ${BASEDIR}/cass_defaultconfig.pri
echo "GSOAP_BIN       = "${GSOAPDIR}"/bin/soapcpp2" >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG         *= DOCS"                     >> ${BASEDIR}/cass_defaultconfig.pri
echo "DOXYGEN_BIN     = "${DOXYDIR}"/bin/doxygen" >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG         *= hdf5"                 >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   *= "${HDF5DIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   *= "${HDF5DIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR *= "${HDF5DIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG       *= singleparticle_hit"    >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR *= "${VIGRADIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG         *= cernroot"                     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   *= "${ROOTSYS}"/include"         >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   *= "${ROOTSYS}"/lib"             >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR *= "${ROOTSYS}"/lib"             >> ${BASEDIR}/cass_defaultconfig.pri
echo "ROOTCINT_BIN    = "${ROOTSYS}"/bin/rootcint"    >> ${BASEDIR}/cass_defaultconfig.pri
echo "ROOTCONFIG_BIN  = "${ROOTSYS}"/bin/root-config" >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG         *= fftw"                 >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   *= "${FFTWDIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   *= "${FFTWDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR *= "${FFTWDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG         *= JoCASSView"          >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_INCDIR   *= "${QWTDIR}"/include" >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LIBDIR   *= "${QWTDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_RPATHDIR *= "${QWTDIR}"/lib"     >> ${BASEDIR}/cass_defaultconfig.pri

echo "CONFIG *= LuCASSView" >> ${BASEDIR}/cass_defaultconfig.pri

echo "QMAKE_CXXFLAGS += -fopenmp"            >> ${BASEDIR}/cass_defaultconfig.pri
echo "QMAKE_LFLAGS   += -fopenmp"            >> ${BASEDIR}/cass_defaultconfig.pri
echo "DEFINES        += _GLIBCXX_PARALLEL"   >> ${BASEDIR}/cass_defaultconfig.pri





# make a clean build of the binaries and install them in the right location for
# the users to run the latest stable version of CASS
cd ${BASEDIR} && qmake PREFIX=${INSTLOC} && make && make install

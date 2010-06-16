# Copyright (C) 2010 Lutz Foucar

# if no file named cass_myconfig.pri exists, this file
# will be read by all .pro files, so it contains all commonly used
# config parameters.
#
# It is good practice to copy this file to cass_myconfig.pri and modify
# this file to your needs. It overrides the default and doesn't need
# to be checked in into the main repository.

# if INSTALLBASE has not been set by the user, set a default value
isEmpty ( INSTALLBASE ){
 INSTALLBASE = ~/installs
}

CONFIG      += release
#CONFIG      += debug
CONFIG      += thread
CONFIG      += warn_on
CONFIG      += exceptions
CONFIG      += rtti
CONFIG      += sse2
CONFIG      += stl
CONFIG      += silent
CONFIG      += static
CONFIG      += staticlib

# Uncomment this if you want to read files instead of connecting to shared LCLS memory
#CONFIG      += offline

# Uncomment the following line to enable pp1000 (HDF5 output)
#CONFIG      += hdf5

# Uncomment this if you want to compile and use the single-particle hit detection postprocessors
#CONFIG      += singleparticle_hit

# Uncomment the following line to enable ROOT Conversion of cass histograms
#CONFIG      += cernroot

# Uncomment the following line and set the path to your QWT header files,
# if they are not found directly under /usr/include.
#QWTINCDIR=/usr/include/qwt

SUFFIX_STR =

CONFIG(debug, debug|release) {
    DEFINES += DEBUG VERBOSE QT_DEBUG
    SUFFIX_STR = _d
}
else {
    DEFINES += NDEBUG QT_NO_DEBUG
}

CONFIG(offline) {
    DEFINES += OFFLINE RINGBUFFER_BLOCKING
}

CONFIG(hdf5) {
    DEFINES += HDF5
}

CONFIG(singleparticle_hit) {
    DEFINES += SINGLEPARTICLE_HIT
}

CONFIG(cernroot) {
    DEFINES += CERNROOT
}

QMAKE_CXXFLAGS_RELEASE += -ftree-vectorize -g -O3 -mtune=nocona -mfpmath=sse,387
QMAKE_CXXFLAGS_DEBUG   += -g -mtune=nocona
QMAKE_LFLAGS +=
QMAKE_STRIP  =

MOC_DIR      = moc
OBJECTS_DIR  = obj$${SUFFIX_STR}

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

bin.path     = $$INSTALLBASE/bin
libs.path    = $$INSTALLBASE/lib
headers.path = $$INSTALLBASE/include




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

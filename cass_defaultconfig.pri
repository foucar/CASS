# Copyright (C) 2010, 2011, 2012, 2013 Lutz Foucar
# Copyright (C) 2011 Stephan Kassemeyer

# if no file named cass_myconfig.pri exists, this file
# will be read by all .pro files, so it contains all commonly used
# config parameters.
#
# It is good practice to copy this file to cass_myconfig.pri and modify
# this file to your needs. It overrides the default and doesn't need
# to be checked in into the main repository.

# if INSTALLBASE has not been set by the user, set a default value
isEmpty ( PREFIX ){
 PREFIX = ~/installs
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

# Uncomment this to build the cass_offline version to read files
CONFIG      += offline

# Uncomment this to build the cass_online version to read files
CONFIG      += online


# Uncomment the following line to enable HDF5 output. If H5 is installed in a
# non default location tell qmake where to find the hdf5 libraries and includes
#CONFIG         += hdf5
#QMAKE_INCDIR   += /path/to/hdf5/include
#QMAKE_LIBDIR   += /path/to/hdf5/lib
#QMAKE_RPATHDIR += /path/to/hdf5/lib

# Uncomment this if you want to compile and use the single-particle hit
# detection postprocessors. If the vigra include files are not in a default
# location tell qmake here where to find it.
#CONFIG         += singleparticle_hit
#QMAKE_INCDIR   += /path/to/vigra/include

# Uncomment the following line to enable ROOT Conversion of cass histograms.
# Let qmake know where to find the ROOT libs and include files. You need to
# ensure that your PATH environmental variable contains the path to the rootcint
# executable, as this is needed for compiling root.
#CONFIG         += cernroot
#QMAKE_INCDIR   += /path/to/root/include
#QMAKE_LIBDIR   += /path/to/root/lib
#QMAKE_RPATHDIR += /path/to/root/lib

# Uncomment the following line to enable a http Server. Let qmake know where
# to find the microhttpd and jpeg libs and include files.
# NOTE: Doesn't work currently
#CONFIG         += httpServer
#QMAKE_INCDIR   += /path/to/microhttp/include
#QMAKE_INCDIR   += /path/to/jpeg/include
#QMAKE_LIBDIR   += /path/to/microhttp/lib
#QMAKE_LIBDIR   += /path/to/jpeg/lib
#QMAKE_RPATHDIR += /path/to/microhttp/lib
#QMAKE_RPATHDIR += /path/to/jpeg/lib

# Uncomment the following to also build the JoCASSViewer
CONFIG          += JoCASSView
#QMAKE_INCDIR   += /path/to/qwt/include
#QMAKE_LIBDIR   += /path/to/qwt/lib
#QMAKE_RPATHDIR += /path/to/qwt/lib

# Uncomment the following to also build the LuCASSViewer. If root is not
# installed in default location and has variables have not been set in cernroot
# section, they need to be set here.
#CONFIG         += LuCASSView
#QMAKE_INCDIR   += /path/to/root/include
#QMAKE_LIBDIR   += /path/to/root/lib
#QMAKE_RPATHDIR += /path/to/root/lib


# Uncomment the following to enable profiling with gprof
#QMAKE_CXXFLAGS  += -pg
#QMAKE_CFLAGS    += -pg
#QMAKE_LFLAGS    += -pg

# Uncomment the following if you want openmp enabled
#QMAKE_CXXFLAGS += -fopenmp
#QMAKE_LFLAGS   += -fopenmp
#DEFINES        += _GLIBCXX_PARALLEL   #enables openmp support for gcc stdlibc++

# additional compiler / linker flags
QMAKE_CXXFLAGS_RELEASE += -ftree-vectorize -g -O3 -mtune=nocona -mfpmath=sse,387
QMAKE_CXXFLAGS_DEBUG   += -g -mtune=nocona
QMAKE_LFLAGS +=
QMAKE_STRIP  =

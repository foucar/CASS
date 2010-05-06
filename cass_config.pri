# Copyright (C) 2010 Lutz Foucar

# this file will be read by all .pro files, so it contains all commonly used
# config parameters

# if INSTALLBASE has not been set by the user, set a default value
isEmpty ( INSTALLBASE ){
 INSTALLBASE = ~/installs
}

CONFIG      += release
CONFIG      += thread
CONFIG      += warn_on
CONFIG      += exceptions
CONFIG      += rtti
CONFIG      += sse2
CONFIG      += stl
#CONFIG      += silent
CONFIG      += static
CONFIG      += staticlib

SUFFIX_STR =

CONFIG(debug, debug|release) {
    DEFINES += DEBUG VERBOSE QT_DEBUG
    SUFFIX_STR = _d
}
else {
    DEFINES += NDEBUG QT_NO_DEBUG
}

QMAKE_CXXFLAGS_RELEASE += -ftree-vectorize -g -O3 -march=opteron -mfpmath=sse,387
QMAKE_CXXFLAGS_DEBUG   += -g -O -march=opteron
QMAKE_LFLAGS +=
QMAKE_STRIP  =

MOC_DIR      = moc
OBJECTS_DIR  = obj$${SUFFIX_STR}

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

VERSION      = 0.1.0

CODECFORTR   = UTF-8

bin.path     = $$INSTALLBASE/bin
libs.path    = $$INSTALLBASE/lib
headers.path = $$INSTALLBASE/include




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

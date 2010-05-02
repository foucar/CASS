# Copyright (C) 2010 Lutz Foucar

#this file will be read by all .pro files, so it contains all commonly used
#config parameters

INSTALLBASE  = ~/installs

CONFIG      += release

CONFIG      += thread
CONFIG      += warn_on
CONFIG      += exceptions
CONFIG      += rtti
CONFIG      += sse2
CONFIG      += stl
CONFIG      += static
CONFIG      += staticlib

QMAKE_STRIP  =

MOC_DIR      = moc
OBJECTS_DIR  = obj

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

VERSION      = 0.1.0

CODECFORTR   = UTF-8

bin.path     = $$INSTALLBASE/bin
libs.path     = $$INSTALLBASE/lib
headers.path = $$INSTALLBASE/include

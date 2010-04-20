# Copyright (C) 2009 Jochen Küpper
# Copyright (C) 2009,2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_machinedata
CONFIG        += release
CONFIG        += thread warn_on exceptions rtti sse2 stl
CONFIG        += static staticlib
QT            -= core gui

CODECFORTR     = UTF-8
DEFINES       += CASS_MACHINEDATA_LIBRARY
INCLUDEPATH   += ../LCLS ../cass
MOC_DIR        = ./obj
OBJECTS_DIR    = ./obj
QMAKE_STRIP    =
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += cass
VERSION        = 0.1.0

OBJECTS_DIR    = ./obj
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o \
	         libcass_machinedata.a

SOURCES += machine_analysis.cpp \
           machine_converter.cpp \


HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/serializer.h \
           machine_analysis.h \
           machine_converter.h \
           cass_machine.h \
           machine_device.h \


header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
header.files   = $$HEADERS
libs.files     = libcass_machinedata.a
INSTALLS      += header libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

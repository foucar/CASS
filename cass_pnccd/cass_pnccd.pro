# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 Nils Kimmel
# Copyright (C) 2009 Nicola Coppola
# Copyright (C) 2009,2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_pnccd
CONFIG        += debug #release
CONFIG        += thread warn_on exceptions rtti sse2 stl
CONFIG        += static staticlib
QT            -= core gui

CODECFORTR     = UTF-8
DEFINES       += CASS_PNCCD_LIBRARY
INCLUDEPATH   +=  ../LCLS ../cass
MOC_DIR        = ./obj
OBJECTS_DIR    = ./obj
QMAKE_STRIP    =
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += cass
VERSION        = 0.1.0

SOURCES       += pnccd_analysis.cpp \
                 pnccd_converter.cpp \

HEADERS       += ../cass/analysis_backend.h \
                 ../cass/parameter_backend.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/ccd_detector.h \
                 ../cass/serializer.h \
                 ../cass/pixel_detector.h \
                 pnccd_analysis.h \
                 pnccd_converter.h \
                 cass_pnccd.h \
                 pnccd_device.h

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009,2010 Lutz Foucar

CONFIG        += static
QT            -= core gui
TEMPLATE       = lib
TARGET         = cass_ccd
DEFINES       += CASS_CCD_LIBRARY
VERSION        = 0.0.1

OBJECTS_DIR    = ./obj
QMAKE_CLEAN    = $$OBJECTS_DIR/*.o \
                 libcass_ccd.a

SOURCES       += ccd_analysis.cpp \
                 ccd_converter.cpp

HEADERS       += ../cass/analysis_backend.h \
                 ../cass/parameter_backend.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/ccd_detector.h \
                 ../cass/serializer.h \
                 ccd_analysis.h \
                 ccd_converter.h \
                 cass_ccd.h \
                 ccd_device.h

INCLUDEPATH   += ../LCLS ../cass 

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
header.files   = $$HEADERS
libs.files     = libcass_ccd.a
INSTALLS      += header libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

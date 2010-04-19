# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 Nils Kimmel
# Copyright (C) 2009 Nicola Coppola
# Copyright (C) 2009,2010 Lutz Foucar

CONFIG        += static staticlib
QT            -= core gui
TARGET         = cass_pnccd
TEMPLATE       = lib

DEFINES       += CASS_PNCCD_LIBRARY
INCLUDEPATH   +=  ../LCLS ../cass
OBJECTS_DIR    = ./obj
VERSION        = 0.1.0

SOURCES       += pnccd_analysis.cpp \
                 pnccd_converter.cpp \

HEADERS       += ../cass/analysis_backend.h \
                 ../cass/parameter_backend.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/ccd_detector.h \
                 ../cass/serializer.h \
                 pnccd_analysis.h \
                 pnccd_converter.h \
                 cass_pnccd.h \
                 pnccd_device.h

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_ccd

CASS_ROOT = ../

include($${CASS_ROOT}/cass_config.pri )

QT            -= core gui

DEFINES       += CASS_CCD_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass

SOURCES       += ccd_analysis.cpp \
                 ccd_converter.cpp

HEADERS       += ../cass/analysis_backend.h \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/ccd_detector.h \
                 ../cass/pixel_detector.h \
                 ../cass/serializer.h \
                 ccd_analysis.h \
                 ccd_converter.h \
                 cass_ccd.h \
                 ccd_device.h

headers.files   = $$HEADERS
libs.files     = libcass_ccd.a
INSTALLS      += headers libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

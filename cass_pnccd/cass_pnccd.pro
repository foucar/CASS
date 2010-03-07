# Copyright (C) 2009 jk, nik, ncoppola, lmf

CONFIG += static
QT -= core gui
TEMPLATE = lib
TARGET = cass_pnccd
DEFINES += CASS_PNCCD_LIBRARY
VERSION = 0.0.1

OBJECTS_DIR = ./obj

SOURCES += pnccd_analysis.cpp \
           pnccd_converter.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/device_backend.h \
           ../cass/ccd_detector.h \
           ../cass/serializer.h \
           pnccd_analysis.h \
           pnccd_converter.h \
           cass_pnccd.h \
           pnccd_device.h \
  
INCLUDEPATH +=  $$(LCLSSYSINCLUDE) \
               ../cass \



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

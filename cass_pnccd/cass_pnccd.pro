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
           ./classes/event/pnccd_event.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           pnccd_analysis.h \
           pnccd_converter.h \
           cass_pnccd.h \
           ./classes/event/pnccd_event.h \
           ./classes/event/pnccd_detector/pnccd_detector.h \
  
INCLUDEPATH += ./ \
               ../cass \
               ./classes/event \
               ./classes/event/pnccd_detector \
               $$(LCLSSYSINCLUDE) \
               ./pnccd_lib \
               ../cass_remi/classes/event \
               ../cass_remi/classes/event/channel \
               ../cass_remi/classes/event/peak \
               ../cass_remi/classes/event/detector


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

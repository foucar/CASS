# Copyright (C) 2009 nik, ncoppola, lmf

CONFIG += shared
#CONFIG += static
QT -= core gui
TEMPLATE = lib
TARGET = cass_pnccd
DEFINES += CASS_PNCCD_LIBRARY
VERSION = 0.0.1

OBJECTS_DIR = ./obj

incFile = $$(QTROOTSYSDIR)/include
exists ($$incFile) {
  include ($$incFile/rootcint.pri)
}

!exists ($$incFile) {
  incFile = $$(ROOTSYS)/include/rootcint.pri
  exists ($$incFile) {
    include ($$incFile)
  }
}

SOURCES += pnccd_analysis.cpp \
           pnccd_converter.cpp \
           ./classes/event/pnccd_event.cpp \
           ./pnccd_lib/frame_data.C \
           ./pnccd_lib/pix_event_data.C

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           pnccd_analysis.h \
           pnccd_converter.h \
           cass_pnccd.h \
           ./classes/event/pnccd_event.h \
           ./classes/event/pnccd_detector/pnccd_detector.h \
           ./pnccd_lib/fformat.h \
           ./pnccd_lib/pef_definition.h \
           ./pnccd_lib/xonline_constants.h \
           ./pnccd_lib/xonline_data_types.h \
           ./pnccd_lib/frame_data.h \
           ./pnccd_lib/pix_event_data.h


INCLUDEPATH += ./ \
               ../cass \
               ./classes/event \
               ./classes/event/pnccd_detector \
               $$(LCLSSYSINCLUDE)


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

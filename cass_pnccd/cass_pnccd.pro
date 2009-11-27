# Copyright (C) 2009 jk, nik, ncoppola, lmf

CONFIG += shared
#CONFIG += static
QT -= core gui
TEMPLATE = lib
TARGET = cass_pnccd
DEFINES += CASS_PNCCD_LIBRARY
VERSION = 0.0.1

OBJECTS_DIR = ./obj

#QMAKE_CFLAGS += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_CXXFLAGS += -pg

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
           ./pnccd_lib/pix_event_data.C \
           ./pnccd_lib/dark_frame_caldata.C \
           ./pnccd_lib/badpix_map_edit.C \
           ./pnccd_lib/pnccd_analysis_lib.cpp

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
           ./pnccd_lib/pix_event_data.h \
           ./pnccd_lib/dark_frame_caldata.h \
           ./pnccd_lib/badpix_map_edit.h \
           ./pnccd_lib/pnccd_analysis_lib.h \
           ./pnccd_lib/pixel_rearr_set.h \
           ./pnccd_lib/pixel_rearrangement.h \
           ./pnccd_lib/pixel_rearr_set.C \
           ./pnccd_lib/pixel_rearrangement.C

INCLUDEPATH += ./ \
               ../cass \
               ./classes/event \
               ./classes/event/pnccd_detector \
               $$(LCLSSYSINCLUDE) \
               ./pnccd_lib


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

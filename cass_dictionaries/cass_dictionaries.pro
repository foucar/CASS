# Copyright (C) 2009 Nicola Coppola

CONFIG += shared static
CONFIG += create_prl
TEMPLATE = lib
qt += core gui
#QT -= core gui
TARGET = cass_dictionaries 
DEFINES += CASS_DICTIONARIES_LIBRARY

VERSION = 0.0.1

CODECFORTR = UTF-8

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


SOURCES += machine_event_dict.cxx \
           peak_dict.cxx \
           channel_dict.cxx \
           detector_dict.cxx \
           remi_event_dict.cxx \
           vmi_event_dict.cxx \
           pnccd_event_dict.cxx \
#           cass_event_dict.cxx

HEADERS += machine_event_dict.h \
           peak_dict.h \
           channel_dict.h \
           detector_dict.h \
           remi_event_dict.h \
           vmi_event_dict.h \
           pnccd_event_dict.h \
           cass_event_dict.h

INCLUDEPATH += ../cass_machinedata/classes/event \
               ../cass_remi/classes/event/peak \
               ../cass_remi/classes/event/channel \
               ../cass_remi/classes/event/detector \
               ../cass_remi/classes/event \
               ../cass_vmi/classes/event \
               ../cass_pnccd/classes/event \
               ../cass \
               $$(LCLSSYSINCLUDE) \


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_dictionaries.so
INSTALLS      += header libs

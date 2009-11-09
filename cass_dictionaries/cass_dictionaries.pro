# Copyright (C) 2009 Nicola Coppola

CONFIG += shared
#CONFIG += static
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

#mytargeta.target = Makefile.dic
#mytargeta.commands = cd ../cass_vmi/classes/event/;rootcint -f vmi_event_dict.cxx -c vmi_event.h LinkDef.h
                     

#mytargeta.commands = cd ;../cre_dict
#QMAKE_EXTRA_TARGETS += mytargeta
#CONFIG += QMAKE_EXTRA_TARGETS

SOURCES += machine_event_dict.cxx \
#           machine_event.cpp \
           peak_dict.cxx \
#           peak.cpp \
           channel_dict.cxx \
#           channel.cpp \
           detector_dict.cxx \
#           detector.cpp \
           remi_event_dict.cxx \
#           remi_event.cpp \
           vmi_event_dict.cxx \
#           vmi_event.cpp \
           pnccd_event_dict.cxx \
           pnccd_detector_dict.cxx \
#           pnccd_event.cpp


HEADERS += machine_event_dict.h \
           peak_dict.h \
           channel_dict.h \
           detector_dict.h \
           remi_event_dict.h \
           vmi_event_dict.h \
           pnccd_event_dict.h \
           pnccd_detector_dict.h \
           cass_event_dict.h

INCLUDEPATH += ../cass_machinedata/classes/event \
               ../cass_remi/classes/event/peak \
               ../cass_remi/classes/event/channel \
               ../cass_remi/classes/event/detector \
               ../cass_remi/classes/event \
               ../cass_vmi/classes/event \
               ../cass_pnccd/classes/event \
               ../cass_pnccd/classes/event/pnccd_detector \
               ../cass \
               $$(LCLSSYSINCLUDE) \


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_dictionaries.so libcass_dictionaries.a
INSTALLS      += header libs

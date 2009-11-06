# Copyright (C) 2009 Jochen Küpper

#CONFIG += shared
CONFIG += static 
QT -= core gui
TEMPLATE = lib
TARGET = cass_machinedata
DEFINES += CASS_MACHINEDATA_LIBRARY
VERSION = 0.0.1

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

SOURCES += machine_analysis.cpp \
           machine_converter.cpp \
           ./classes/event/machine_event.cpp \


HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           machine_analysis.h \
           machine_converter.h \
           cass_machine.h \
           ./classes/event/machine_event.h \

INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \
               ./classes/event \




INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_machinedata.a
INSTALLS      += header libs

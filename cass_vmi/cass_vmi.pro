# Copyright (C) 2009 Jochen Küpper
#modified Copyright (C) 2009 N Coppola

CONFIG += static 
CONFIG += create_prl
QT -= core gui
TEMPLATE = lib
TARGET = cass_vmi
#TARGET += classes/event/vmi_event_dict.cxx
DEFINES += CASS_VMI_LIBRARY
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

SOURCES += vmi_analysis.cpp \
           vmi_converter.cpp \
           ./classes/event/vmi_event.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           vmi_analysis.h \
           vmi_converter.h \
           cass_vmi.h \
           ./classes/event/vmi_event.h \

INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \
               ./classes/event \




INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_vmi.a
INSTALLS      += header libs

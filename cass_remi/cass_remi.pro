# Copyright (C) 2009 Jochen Küpper
#modified Copyright (C) 2009 N Coppola,lmf
#CONFIG += shared
CONFIG += static
CONFIG += create_prl
QT -= core gui
TEMPLATE = lib
DEFINES += CASS_REMI_LIBRARY

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

TARGET = cass_remi
TARGET += libcass_remi.a

SOURCES += remi_analysis.cpp \
           remi_converter.cpp \
           ./classes/event/remi_event.cpp \
           ./classes/event/channel/channel.cpp \
           ./classes/event/peak/peak.cpp \
           ./classes/event/detector/detector.cpp \
           ./classes/waveformanalyzer/cfd.cpp \
           ./classes/waveformanalyzer/com.cpp \
           ./classes/detektorhitsorter/detektorhitsorter.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_quad.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_simple.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           remi_analysis.h \
           remi_converter.h \
           cass_remi.h  \
           ./classes/event/remi_event.h \
           ./classes/event/channel/channel.h \
           ./classes/event/peak/peak.h \
           ./classes/event/detector/detector.h \
           ./classes/waveformanalyzer/waveform_analyzer.h \
           ./classes/waveformanalyzer/cfd.h \
           ./classes/waveformanalyzer/com.h \
           ./classes/waveformanalyzer/helperfunctionsforstdc.h \
           ./classes/detektorhitsorter/detektorhitsorter.h \
           ./classes/detektorhitsorter/detektorhitsorter_quad.h \
           ./classes/detektorhitsorter/detektorhitsorter_simple.h \

INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \
               ./classes/waveformanalyzer \
               ./classes/detektorhitsorter \
               ./classes/event \
               ./classes/event/channel \
               ./classes/event/peak \
               ./classes/event/detector \
               ./

INSTALLBASE     = /usr/local/cass
header.path     = $$INSTALLBASE/include
libs.path       = $$INSTALLBASE/libs

header.files    = $$HEADERS
libs.files      = libcass_remi*
INSTALLS        += header libs

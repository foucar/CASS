# Copyright (C) 2009 Jochen Küpper

CONFIG += static

QT -= core gui

VERSION = 0.0.1

TEMPLATE = lib

TARGET = cass_remi

SOURCES += REMIAnalysis.cpp \
           ./Classes/Event/REMIEvent.cpp \
           ./Classes/Event/Channel/Channel.cpp \
           ./Classes/Event/Peak/Peak.cpp \
           ./Classes/Event/Detector/Detector.cpp \
           ./Classes/SignalAnalyzer/SignalAnalyzer.cpp \
           ./Classes/SignalAnalyzer/SoftTDCCFD.cpp \
           ./Classes/SignalAnalyzer/SoftTDCCoM.cpp \
           ./Classes/SignalAnalyzer/helperfunctionsForSTDC.cpp\
           ./Classes/DetektorHitSorter/DetektorHitSorter.cpp \
           ./Classes/DetektorHitSorter/DetektorHitSorterQuad.cpp \
           ./Classes/DetektorHitSorter/DetektorHitSorterSimple.cpp \
           ./Classes/LCLS/pdsdata/acqiris/src/ConfigV1.cc \
           ./Classes/LCLS/pdsdata/acqiris/src/DataDescV1.cc

HEADERS += REMIAnalysis.h \
           cass_remi.h  \
           ./Classes/Event/REMIEvent.h \
           ./Classes/Event/Channel/Channel.h \
           ./Classes/Event/Peak/Peak.h \
           ./Classes/Event/Detector/Detector.h \
           ./Classes/SignalAnalyzer/SignalAnalyzer.h \
           ./Classes/SignalAnalyzer/SoftTDC.h \
           ./Classes/SignalAnalyzer/SoftTDCCFD.h \
           ./Classes/SignalAnalyzer/SoftTDCCoM.h \
           ./Classes/SignalAnalyzer/helperfunctionsForSTDC.h \
           ./Classes/DetektorHitSorter/DetektorHitSorter.h \
           ./Classes/DetektorHitSorter/DetektorHitSorterQuad.h \
           ./Classes/DetektorHitSorter/DetektorHitSorterSimple.h \
           ./Classes/LCLS/pdsdata/acqiris/ConfigV1.hh \
           ./Classes/LCLS/pdsdata/acqiris/DataDescV1.hh

INCLUDEPATH += ../cass \
               ./Classes/LCLS \
               ./Classes/SignalAnalyzer \
               ./Classes/DetektorHitSorter \
               ./Classes/Event \
               ./Classes/Event/Channel \
               ./Classes/Event/Peak \
               ./Classes/Event/Detector

INSTALLBASE     = /usr/local/cass
header.path     = $$INSTALLBASE/include
libs.path       = $$INSTALLBASE/libs

header.files    = $$HEADERS
libs.files      = libcass_remi.a
INSTALLS        += header libs

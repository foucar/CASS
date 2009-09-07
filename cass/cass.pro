# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

TEMPLATE = lib

TARGET = cass

DEFINES += CASS_LIBRARY

VERSION = 0.0.1

SOURCES += AnalysisBackend.cpp \
           Analyzer.cpp \
           EventQueue.cpp \
           FormatConverter.cpp \
           RootTree.cpp \
           cass.cpp \
           ../LCLS/pdsdata/app/XtcMonitorClient.cc

HEADERS += AnalysisBackend.h \
           Analyzer.h \
           ConversionBackend.h \
           Event.h \
           EventQueue.h \
           FormatConverter.h \
           Image.h \
           RootTree.h \
           cass.h \
           ../LCLS/pdsdata/app/XtcMonitorClient.hh

INCLUDEPATH += ../LCLS \
               ../cass_REMI/Classes/Event \
               ../cass_REMI/Classes/Event/Channel \
               ../cass_REMI/Classes/Event/Peak \
               ../cass_REMI/Classes/Event/Detector \
               ../cass_VMI/Classes/Event \

bin.path       = $$INSTALLBASE/bin
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

bin.files      = cass
header.files   = $$HEADERS
libs.files     = libcass.a
INSTALLS      += header libs bin

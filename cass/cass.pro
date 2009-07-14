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
           cass.cpp

HEADERS += AnalysisBackend.h \
           Analyzer.h \
           Event.h \
           EventQueue.h \
           FormatConverter.h \
           Image.h \
           RootTree.h \
           cass.h

INCLUDEPATH += ../LCLS \
               ../cass_REMI/Classes/Event \
               ../cass_REMI/Classes/Event/Channel \
               ../cass_REMI/Classes/Event/Peak \
               ../cass_REMI/Classes/Event/Detector

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass.a
INSTALLS      += header libs

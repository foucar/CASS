# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

TEMPLATE = lib

TARGET = cass

DEFINES += CASS_LIBRARY

VERSION = 0.0.1

SOURCES += AnalysisBackend.cpp

HEADERS += AnalysisBackend.h \
           cass.h \
           Event.h \
           Image.h

INCLUDEPATH += ../cass_REMI/Classes/LCLS \
               ../cass_REMI/Classes/Event \
               ../cass_REMI/Classes/Event/Channel \
               ../cass_REMI/Classes/Event/Peak \
               ../cass_REMI/Classes/Event/Detector

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass.a
INSTALLS      += header libs

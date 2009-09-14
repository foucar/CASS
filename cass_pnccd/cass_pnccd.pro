# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_pnCCD

VERSION = 0.0.1


SOURCES += pnCCDAnalysis.cpp \
           pnCCDConverter.cpp

HEADERS += pnCCDAnalysis.h \
           pnCCDConverter.h \
           cass_pnccd.h \
           ./Classes/Event/pnCCDEvent.h


INCLUDEPATH += ../cass ./Classes/Event


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnCCD*.a
INSTALLS      += header libs

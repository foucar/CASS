# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_pnCCD

VERSION = 0.0.1


SOURCES += pnCCDAnalysis.cpp

HEADERS += pnCCDAnalysis.h \
           cass_pnccd.h


INCLUDEPATH += ../cass


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnCCD*.a
INSTALLS      += header libs

# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_VMI

gVERSION = 0.0.1


INCLUDEPATH += ../cass

SOURCES += VMIAnalysis.cpp

HEADERS += VMIAnalysis.h \
           cass_pnccd.h


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_VMI.a
INSTALLS      += header libs

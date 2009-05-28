# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

VERSION = 0.0.1

TEMPLATE = lib


TARGET = cass_remi

SOURCES += REMIAnalysis.cpp

HEADERS += REMIAnalysis.h \
           cass_remi.h


INCLUDEPATH += ../cass


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_remi.a
INSTALLS      += header libs

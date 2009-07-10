# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_VMI

gVERSION = 0.0.1


INCLUDEPATH += ../cass \
               ../LCLS \
               ./Classes/Event \

SOURCES += VMIAnalysis.cpp \
           ./Classes/Event/VMIEvent.cpp \


HEADERS += VMIAnalysis.h \
           cass_vmi.h \
           ./Classes/Event/VMIEvent.h



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_VMI.a
INSTALLS      += header libs

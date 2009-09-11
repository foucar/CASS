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
           VMIConverter.cpp \
           ./Classes/Event/VMIEvent.cpp \
           ../LCLS/pdsdata/camera/src/FrameV1.cc


HEADERS += VMIAnalysis.h \
           VMIConverter.h \
           cass_vmi.h \
           ./Classes/Event/VMIEvent.h \
           ../LCLS/pdsdata/camera/FrameV1.hh



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_VMI.a
INSTALLS      += header libs

# Copyright (C) 2009 Jochen Küpper

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_vmi

gVERSION = 0.0.1


INCLUDEPATH += ../cass \
               ../LCLS \
               ./classes/event \

SOURCES += vmi_analysis.cpp \
           vmi_converter.cpp \
           ./classes/event/vmi_event.cpp \
           ../LCLS/pdsdata/camera/src/FrameV1.cc


HEADERS += vmi_analysis.h \
           vmi_converter.h \
           cass_vmi.h \
           ./classes/event/vmi_event.h \
           ../LCLS/pdsdata/camera/FrameV1.hh



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_vmi.a
INSTALLS      += header libs

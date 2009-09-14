# Copyright (C) 2009 lutz foucar

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_imageprocessor

gVERSION = 0.0.1


INCLUDEPATH += ../cass

SOURCES += image_processor.cpp

HEADERS += image_processor.h \
           cass_imageprocessor.h


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_imageprocessor.a
INSTALLS      += header libs

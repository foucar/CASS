# Copyright (C) 2009 lutz foucar

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_ImageProcessor

gVERSION = 0.0.1


INCLUDEPATH += ../cass

SOURCES += ImageProcessor.cpp

HEADERS += ImageProcessor.h \
           cass_imageprocessor.h


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_ImageProcessor.a
INSTALLS      += header libs

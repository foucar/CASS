# Copyright (C) 2009 lutz foucar

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_database

gVERSION = 0.0.1


INCLUDEPATH += ../cass

SOURCES += database.cpp

HEADERS += database.h \
           cass_database.h

INCLUDEPATH += ../LCLS \
               ../cass_remi/classes/event \
               ../cass_remi/classes/event/channel \
               ../cass_remi/classes/event/peak \
               ../cass_remi/classes/event/detector



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_database.a
INSTALLS      += header libs

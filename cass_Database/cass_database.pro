# Copyright (C) 2009 lutz foucar

CONFIG += static 

QT -= core gui

TEMPLATE = lib


TARGET = cass_database

gVERSION = 0.0.1


INCLUDEPATH += ../cass

SOURCES += Database.cpp

HEADERS += Database.h \
           cass_database.h

INCLUDEPATH += ../cass_REMI/Classes/LCLS \
               ../cass_REMI/Classes/Event \
               ../cass_REMI/Classes/Event/Channel \
               ../cass_REMI/Classes/Event/Peak \
               ../cass_REMI/Classes/Event/Detector



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_database.a
INSTALLS      += header libs

# Copyright (C) 2009 lutz foucar

CONFIG += static 
QT -= core gui
TEMPLATE = lib
TARGET = cass_database
DEFINES += CASS_DATABASE_LIBRARY

VERSION = 0.0.1



SOURCES += database.cpp

HEADERS += database.h \
           cass_database.h

INCLUDEPATH += ../cass


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_database.a
INSTALLS      += header libs

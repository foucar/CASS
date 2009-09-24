# Copyright (C) 2009 lutz foucar
# modified Copyright (C) 2009 N Coppola

CONFIG += static 
#the following could be removed in the future...
CONFIG += create_prl
TEMPLATE = lib
qt += core gui
#QT -= core gui
TARGET = cass_database
DEFINES += CASS_DATABASE_LIBRARY

VERSION = 0.0.1

CODECFORTR = UTF-8

incFile = $$(QTROOTSYSDIR)/include
exists ($$incFile) {
  include ($$incFile/rootcint.pri)
}

!exists ($$incFile) {
  incFile = $$(ROOTSYS)/include/rootcint.pri
  exists ($$incFile) {
    include ($$incFile)
  }
}


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

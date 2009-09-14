# Copyright (C) 2009 Nicola Coppola

CONFIG += static
CONFIG += create_prl

TEMPLATE = lib
qt += core gui

#FORMS += MainWindow.ui
HEADERS +=  circ_root.h
SOURCES += circ_qt.C
           
TARGET = root

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

win32:debug{
LIBS += -L../cass_REMI/Debug -lcass_REMI \
        -L../cass_pnCCD/Debug -lcass_pnCCD \
        -L../cass_VMI/Debug -lcass_VMI \
        -L../cass/Debug -lcass
}
win32:release{
LIBS += -L../cass_REMI/Release -lcass_REMI \
        -L../cass_pnCCD/Release -lcass_pnCCD \
        -L../cass_VMI/Release -lcass_VMI \
        -L../cass/Release -lcass
}
unix{
LIBS += -L../cass_remi -lcass_remi \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_vmi -lcass_vmi \
        -L../cass -lcass
}
INSTALLBASE = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files = $$HEADERS
lib.files = libroot*.a
INSTALLS += header libs

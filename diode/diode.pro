# Copyright (C) 2009 Jochen Küpper

TEMPLATE = app
qt += core gui

FORMS += MainWindow.ui
HEADERS += diode.h \
           ImageHandler.h \
           MainWindow.h \
           ../cass_root/circ_root.h
SOURCES += diode.cpp \
           ImageHandler.cpp \
           MainWindow.cpp \

TARGET = diode

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

INCLUDEPATH += ../cass_root

win32:debug{
LIBS += -L../cass_REMI/Debug -lcass_REMI \
        -L../cass_pnCCD/Debug -lcass_pnCCD \
        -L../cass_VMI/Debug -lcass_VMI \
        -L../cass/Debug -lcass \
        -L../cass_root -l Root
}
win32:release{
LIBS += -L../cass_REMI/Release -lcass_REMI \
        -L../cass_pnCCD/Release -lcass_pnCCD \
        -L../cass_VMI/Release -lcass_VMI \
        -L../cass/Release -lcass \
        -L../cass_root -l Root
}
unix{
LIBS += -L../cass_REMI -lcass_REMI \
        -L../cass_pnCCD -lcass_pnCCD \
        -L../cass_VMI -lcass_VMI \
        -L../cass -lcass \
        -L../cass_root -l Root
}
INSTALLBASE = /usr/local/cass

bin.path = $$INSTALLBASE/bin
bin.files = diode.app
INSTALLS += bin

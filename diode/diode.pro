# Copyright (C) 2009 Jochen Küpper

TEMPLATE = app
qt += core gui

FORMS += MainWindow.ui
HEADERS += diode.h \
           ImageHandler.h \
           MainWindow.h
SOURCES += diode.cpp \
           ImageHandler.cpp \
           MainWindow.cpp \
           ../cass_root/circ_qt.C
# for the moment I am just adding the previous line, in the future I will have a library...

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
LIBS += -L../cass_REMI -lcass_REMI \
        -L../cass_pnCCD -lcass_pnCCD \
        -L../cass_VMI -lcass_VMI \
        -L../cass -lcass
}
INSTALLBASE = /usr/local/cass

bin.path = $$INSTALLBASE/bin
bin.files = diode.app
INSTALLS += bin

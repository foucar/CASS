# Copyright (C) 2009 Jochen Küpper

#CONFIG = debug
#CONFIG += release
CONFIG += create_prl

PRE_TARGETDEPS += Makefile

mytarget.target = Makefile
mytarget.commands = makedepend -f $$mytarget.target diode.cpp
mytarget.depends = mytarget2
LIBS_NAMES += ../cass_REMI/libcass_REMI.a ../cass_pnCCD/libcass_pnCCD.a ../cass_VMI/libcass_VMI.a \
  ../cass/libcass.a ../cass_root/libRoot.a
mytarget2.commands = echo $$LIBS_NAMES>> $$mytarget.target
#mytarget3.commands = @echo Building $$mytarget.target
QMAKE_EXTRA_TARGETS += mytarget mytarget2

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
#DEPENDPATH += ../cass_REMI  \
#        -L../cass_pnCCD \
#        -L../cass_VMI  \
#        -L../cass  \
#        -L../cass_root

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

#for(a, LIBS_NAMES):exists($${a}):message(I see $${a})
#for(a, LIBS_NAMES):system(newer diode $${a}):message(diode newer than $${a})
#POST_TARGETDEPS += $$LIBS_NAMES
#PRE_TARGETDEPS += $$LIBS_NAMES
#depends += $$LIBS_NAMES

}

INSTALLBASE = /usr/local/cass

bin.path = $$INSTALLBASE/bin
bin.files = diode.app
INSTALLS += bin


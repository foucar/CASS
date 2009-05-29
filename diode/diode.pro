# Copyright (C) 2009 Jochen Küpper

TEMPLATE = app
qt += core gui

FORMS += MainWindow.ui
HEADERS += diode.h \
           MainWindow.h
SOURCES += diode.cpp

TARGET = diode

LIBS += -L../cass_pnCCD -lcass_pnCCD \
    -L../cass_REMI -lcass_REMI \
    -L../cass_VMI -lcass_VMI \
    -L../cass -lcass

INSTALLBASE = /usr/local/cass

bin.path = $$INSTALLBASE/bin
bin.files = diode.app
INSTALLS += bin

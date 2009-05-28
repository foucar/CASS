# Copyright (C) 2009 Jochen Küpper

TEMPLATE = app

qt += core gui


HEADERS += diode.h

SOURCES += diode.cpp

TARGET = diode


LIBS += -L../cass_pnCCD -lcass_pnCCD \
        -L../cass_REMI -lcass_REMI \
        -L../cass_VMI -lcass_VMI \
        -L../cass -lcass

bin.path       = $$INSTALLBASE/bin

bin.files      = diode.app
INSTALLS       += bin

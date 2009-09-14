# Copyright (C) 2009 Jochen Küpper
CONFIG += static
TEMPLATE = lib
#TEMPLATE = app
TARGET = cass
DEFINES += CASS_LIBRARY
VERSION = 0.0.1


SOURCES +=  analyzer.cpp \
            event_queue.cpp \
            format_converter.cpp \
            cass.cpp \
            #../LCLS/pdsdata/app/XtcMonitorClient.cc \
            cass_event.cpp \
            xtciterator.cpp

HEADERS +=  analysis_backend.h \
            analyzer.h \
            conversion_backend.h \
            event_queue.h \
            format_converter.h \
            cass.h \
            ../LCLS/pdsdata/app/XtcMonitorClient.hh \
            cass_event.h \
            xtciterator.h

INCLUDEPATH += ./ \
    ../LCLS \
    ../cass_remi \
    ../cass_remi/classes/event \
    ../cass_remi/classes/event/channel \
    ../cass_remi/classes/event/peak \
    ../cass_remi/classes/event/detector \
    ../cass_remi/classes/detektorhitsorter \
    ../cass_remi/classes/signalanalyzer \
    ../cass_vmi \
    ../cass_vmi/classes/event \
    ../cass_pnccd \
    ../cass_pnccd/classes/event

win32:debug{
LIBS += -L../cass_remi/Debug -lcass_remi \
        -L../cass_pnccd/Debug -lcass_pnccd \
        -L../cass_vmi/Debug -lcass_vmi \
        -L../cass/Debug -lcass \
        -L../cass_root -l root
}
win32:release{
LIBS += -L../cass_remi/Release -lcass_remi \
        -L../cass_pnccd/Release -lcass_pnccd \
        -L../cass_vmi/Release -lcass_vmi \
        -L../cass/Release -lcass \
        -L../cass_root -l root
}
unix{
LIBS += -L../cass_remi -lcass_remi \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_vmi -lcass_vmi \
        -L../cass -lcass \
        -L../cass_root -l root
}

bin.path = $$INSTALLBASE/bin
header.path = $$INSTALLBASE/include
libs.path = $$INSTALLBASE/libs
bin.files = cass
header.files = $$HEADERS
libs.files = libcass.a
INSTALLS += header \
    libs \
    bin

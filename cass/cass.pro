# Copyright (C) 2009 Jochen Küpper
CONFIG += static
#TEMPLATE = lib
TEMPLATE = app
TARGET = cass
DEFINES += CASS_LIBRARY
VERSION = 0.0.1
CODECFORTR = UTF-8

SOURCES +=  cass.cpp \
            analyzer.cpp \
            event_queue.cpp \
            format_converter.cpp \
            $$(LCLSSYSINCLUDE)/pdsdata/app/XtcMonitorClient.cc \ #we have to live with this hack until lcls has made this part of their library
            xtciterator.cpp

HEADERS +=  analysis_backend.h \
            analyzer.h \
            conversion_backend.h \
            event_queue.h \
            format_converter.h \
            cass.h \
            cass_event.h \
            xtciterator.h

INCLUDEPATH +=  ./ \
                $$(LCLSSYSINCLUDE) \
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
                ../cass_pnccd/classes/event \
                ../cass_database

win32:debug{
LIBS += -L../cass_remi/Debug -lcass_remi \
        -L../cass_pnccd/Debug -lcass_pnccd \
        -L../cass_vmi/Debug -lcass_vmi \
        -L../cass_database/Debug -lcass_database \
}
win32:release{
LIBS += -L../cass_remi/Release -lcass_remi \
        -L../cass_pnccd/Release -lcass_pnccd \
        -L../cass_vmi/Release -lcass_vmi \
        -L../cass_database/Release -lcass_database \
}
unix{
QMAKE_LFLAGS += -Wl,-rpath,$$(LCLSSYSLIB) -L
LIBS += -L../cass_remi -lcass_remi \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_vmi -lcass_vmi \
#        -L../cass -lcass \
        -L../cass_database -lcass_database \
#        -L../cass_root -lroot
        -L$$(LCLSSYSLIB) -lacqdata -lxtcdata -lpulnixdata -lcamdata
}

INSTALLBASE = /usr/local/cass

bin.path = $$INSTALLBASE/bin
header.path = $$INSTALLBASE/include
libs.path = $$INSTALLBASE/libs
bin.files = cass.app
header.files = $$HEADERS
libs.files = libcass.a

#INSTALLS += bin

INSTALLS += header \
    libs \
    bin

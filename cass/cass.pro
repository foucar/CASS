# Copyright (C) 2009 ncoppola, lmf, Jochen KÃpper
TEMPLATE = app
CONFIG += static
QT += network
TARGET = cass
DEFINES += CASS_LIBRARY
VERSION = 0.1.0
CODECFORTR = UTF-8

OBJECTS_DIR = ./obj
MOC_DIR = ./obj

SOURCES +=  cass.cpp \
            analyzer.cpp \
            sharedmemory_input.cpp \
            format_converter.cpp \
            cass_event.cpp \
            #ratemeter.cpp \
            worker.cpp \
            tcpserver.cpp \
            event_getter.cpp \
            histogram_getter.cpp \
            ./postprocessing/post_processor.cpp

HEADERS +=  analysis_backend.h \
            conversion_backend.h \
            sharedmemory_input.h \
            analyzer.h \
            format_converter.h \
            cass.h \
            cass_event.h \
            xtciterator.h \
            #ratemeter.h \
            ringbuffer.h \
            ccd_detector.h \
            worker.h \
            tcpserver.h \
            event_getter.h \
            histogram_getter.h \
            histogram.h \
            serializer.h \
            ./postprocessing/post_processor.h \

INCLUDEPATH +=  $$(LCLSSYSINCLUDE) \
                ../cass_acqiris \
                ../cass_acqiris/classes \
                ../cass_ccd \
                ../cass_pnccd \
                ../cass_machinedata \
                ./postprocessing \
                ./



unix{
SOURCES += $$(LCLSSYSINCLUDE)/pdsdata/app/XtcMonitorClient.cc # we have to live with this hack until lcls has made this part of their library
QMAKE_LFLAGS += -Wl,-rpath,$$(LCLSSYSLIB)
LIBS += -L../cass_acqiris -lcass_acqiris \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_ccd -lcass_ccd \
        -L../cass_machinedata -lcass_machinedata \
        -L$$(LCLSSYSLIB) -lacqdata -lxtcdata -lpulnixdata -lcamdata -lpnccddata \

TARGETDEPS +=	../cass_acqiris/libcass_acqiris.a \
                ../cass_pnccd/libcass_pnccd.a \
                ../cass_ccd/libcass_ccd.a \
				../cass_machinedata/libcass_machinedata.a \
}


INSTALLBASE = /usr/local/cass
bin.path = $$INSTALLBASE/bin
header.path = $$INSTALLBASE/include
libs.path = $$INSTALLBASE/libs
bin.files = cass.app
header.files = $$HEADERS
libs.files = libcass.a

# INSTALLS += bin
INSTALLS += header \
    libs \
    bin

# Copyright (C) 2009,2010 Jochen Küpper
# Copyright (C) 2009,2010 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

CODECFORTR     = UTF-8
CONFIG        += static staticlib
CONFIG        += thread warn_on exceptions rtti sse2 stl
DEFINES       += CASS_LIBRARY
QT            += network
TARGET         = cass
TEMPLATE       = app
VERSION        = 0.1.0

OBJECTS_DIR = ./obj
MOC_DIR = ./obj

# compile the LCLS libraries before compiling cass itself
lclstarget.target = LCLSLibrary
lclstarget.commands = @echo "creating LCLS Library"; cd ../LCLS && make x86_64-linux ; cd -
lclstarget.depends = FORCE
lclstarget.path     =  $$INSTALLBASE/lib
lclstarget.files    = ../LCLS/build/pdsdata/lib/x86_64-linux/libacqdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libappdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libbld.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libcamdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libcontroldata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libepics.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libevrdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libipimbdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libopal1kdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libpnccddata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libprincetondata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libpulnixdata.so \
                      ../LCLS/build/pdsdata/lib/x86_64-linux/libxtcdata.so
INSTALLS += lclstarget

# SOAPFiles.target = soapStub.h
# SOAPFiles.commands = @echo "creating SOAP Server Files" && soapcpp2 -S -i soapserver.h
# SOAPFiles.depends = FORCE

# PRE_TARGETDEPS += soapStub.h LCLSLibrary
# QMAKE_EXTRA_TARGETS += SOAPFiles lclstarget
PRE_TARGETDEPS      += LCLSLibrary
QMAKE_EXTRA_TARGETS += lclstarget

SOURCES +=  daemon.cpp \
            cass.cpp \
            analyzer.cpp \
            sharedmemory_input.cpp \
            format_converter.cpp \
            cass_event.cpp \
            ratemeter.cpp \
            worker.cpp \
            event_getter.cpp \
            histogram_getter.cpp \
            rate_plotter.cpp \
            ./postprocessing/postprocessor.cpp \
            ./postprocessing/ccd.cpp \
            ./postprocessing/alignment.cpp \
            ./postprocessing/waveform.cpp \
            ./postprocessing/acqiris_detectors.cpp \
            ./postprocessing/imaging.cpp \
            # soapCASSsoapService.cpp \
            # soapC.cpp \
            # tcpserver.cpp

HEADERS +=  analysis_backend.h \
            cass.h \
            cass_event.h \
            conversion_backend.h \
            sharedmemory_input.h \
            analyzer.h \
            format_converter.h \
            histogram.h \
            histogram_getter.h \
            xtciterator.h \
            ratemeter.h \
            ringbuffer.h \
            ccd_detector.h \
            worker.h \
            daemon.h \
            event_getter.h \
            serializable.h \
            serializer.h \
            rate_plotter.h \
            ./postprocessing/postprocessor.h \
            ./postprocessing/backend.h \
            ./postprocessing/ccd.h \
            ./postprocessing/alignment.h \
            ./postprocessing/waveform.h \
            ./postprocessing/acqiris_detectors.h \
            ./postprocessing/imaging.h
            # soapCASSsoapService.h \
            # soapH.h \
            # soapStub.h \
            # tcpserver.h

INCLUDEPATH +=  ../cass_acqiris \
                ../cass_acqiris/classes \
                ../cass_acqiris/classes/detector_analyzer \
                ../cass_acqiris/classes/waveformanalyzer \
                ../cass_ccd \
                ../cass_pnccd \
                ../cass_machinedata \
                ./postprocessing \
                ../LCLS \
                .


unix{
#QMAKE_LFLAGS += -Wl,-rpath,$$(LCLSSYSLIB)
LIBS += -L../cass_acqiris -lcass_acqiris \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_ccd -lcass_ccd \
        -L../cass_machinedata -lcass_machinedata \
        -L../LCLS/build/pdsdata/lib/x86_64-linux \
        -lacqdata -lxtcdata -lpulnixdata -lcamdata -lpnccddata \
        -levrdata -lappdata \
        #-lgsoap++ -lgsoap

TARGETDEPS +=	../cass_acqiris/libcass_acqiris.a \
              ../cass_pnccd/libcass_pnccd.a \
              ../cass_ccd/libcass_ccd.a \
              ../cass_machinedata/libcass_machinedata.a \
}


bin.path       = $$INSTALLBASE/bin
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
bin.files      = cass.app
header.files   = $$HEADERS
libs.files     = libcass.a

INSTALLS      += header libs bin




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

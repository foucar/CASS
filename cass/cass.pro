# Copyright (C) 2009,2010 Jochen Küpper
# Copyright (C) 2009,2010 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE = app
CONFIG += static
QT += network
TARGET = cass
DEFINES += CASS_LIBRARY DWITH_NONAMESPACES

#make the cass project compile the LCLS Library before compiling cass itself
lclstarget.target = LCLSLibrary
lclstarget.commands = @echo "creating LCLS Library"; cd $$(LCLSSYSINCLUDE) ; make x86_64-linux ; cd -
lclstarget.depends = FORCE

SOAPFiles.target = soapStub.h
SOAPFiles.commands = @echo "creating SOAP Server Files" && soapcpp2 -S -i soapserver.h
SOAPFiles.depends = FORCE

PRE_TARGETDEPS += soapStub.h LCLSLibrary
QMAKE_EXTRA_TARGETS += SOAPFiles lclstarget

VERSION = 0.1.0
CODECFORTR = UTF-8

OBJECTS_DIR = ./obj
MOC_DIR = ./obj

SOURCES +=  daemon.cpp \
            cass.cpp \
            analyzer.cpp \
            sharedmemory_input.cpp \
            format_converter.cpp \
            cass_event.cpp \
            ratemeter.cpp \
            worker.cpp \
            tcpserver.cpp \
            event_getter.cpp \
            histogram_getter.cpp \
            rate_plotter.cpp \
            soapCASSsoapService.cpp \
            soapC.cpp \
            ./postprocessing/postprocessor.cpp \
            ./postprocessing/ccd.cpp \
            ./postprocessing/alignment.cpp \
            ./postprocessing/waveform.cpp \
            ./postprocessing/acqiris_detectors.cpp \
            ./postprocessing/imaging.cpp \

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
            soapCASSsoapService.h \
            soapH.h \
            soapStub.h \
            tcpserver.h \
            ./postprocessing/postprocessor.h \
            ./postprocessing/backend.h \
            ./postprocessing/ccd.h \
            ./postprocessing/alignment.h \
            ./postprocessing/waveform.h \
            ./postprocessing/acqiris_detectors.h \
            ./postprocessing/imaging.h \

INCLUDEPATH +=  ../cass_acqiris \
                ../cass_acqiris/classes \
                ../cass_acqiris/classes/detector_analyzer \
                ../cass_acqiris/classes/waveformanalyzer \
                ../cass_ccd \
                ../cass_pnccd \
                ../cass_machinedata \
                ./postprocessing \
                ./
                #$$(LCLSSYSINCLUDE) \


unix{
#QMAKE_LFLAGS += -Wl,-rpath,$$(LCLSSYSLIB)
LIBS += -L../cass_acqiris -lcass_acqiris \
        -L../cass_pnccd -lcass_pnccd \
        -L../cass_ccd -lcass_ccd \
        -L../cass_machinedata -lcass_machinedata \
        #-L$$(LCLSSYSLIB)
        -lacqdata -lxtcdata -lpulnixdata -lcamdata -lpnccddata -levrdata -lappdata \
        -lgsoap++ -lgsoap

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



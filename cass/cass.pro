# Copyright (C) 2009,2010 Jochen Küpper
# Copyright (C) 2009,2010 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE       = app
CONFIG        += static staticlib
CONFIG        += thread warn_on exceptions rtti sse2 stl
QT            += network
TARGET         = cass

CODECFORTR     = UTF-8
DEFINES       += CASS_LIBRARY
VERSION        = 0.1.0

OBJECTS_DIR    = ./obj
MOC_DIR        = ./obj
QMAKE_CLEAN   += ${OBJECTS_DIR}/*.o
QMAKE_CLEAN   += ${MOC_DIR}/moc_*

# compile the LCLS libraries before compiling cass itself
lclstarget.target   = LCLSLibrary
lclstarget.commands = cd ../LCLS && make x86_64-linux ; cd -
lclstarget.depends  = FORCE
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
QMAKE_CLEAN        += $$lclstarget.files
INSTALLS           += lclstarget

SOAPFiles.input     = soapserver.h
SOAPFiles.target    = soapCASSsoapService.cpp
SOAPFiles.commands  = soapcpp2 -S -i soapserver.h
QMAKE_CLEAN        += soapCASSsoapService.cpp soapCASSsoapService.h soapC.cpp soapH.h soapStub.h \
	              CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
		      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
		      ns.xsd CASSsoap.nsmap CASSsoap.wsdl

PRE_TARGETDEPS     += soapCASSsoapService.cpp LCLSLibrary
QMAKE_EXTRA_TARGETS+= SOAPFiles lclstarget

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
	    soapCASSsoapService.cpp \
            soapC.cpp \
	    tcpserver.cpp

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
            ./postprocessing/imaging.h \
            soapCASSsoapService.h \
            soapH.h \
            soapStub.h \
            tcpserver.h

INCLUDEPATH   += postprocessing \
                 ../cass_acqiris \
                 ../cass_acqiris/classes \
                 ../cass_acqiris/classes/detector_analyzer \
                 ../cass_acqiris/classes/waveformanalyzer \
                 ../cass_ccd \
                 ../cass_pnccd \
                 ../cass_machinedata \
                 ../LCLS \
                 .

LIBS          += -L../cass_acqiris -lcass_acqiris \
                 -L../cass_pnccd -lcass_pnccd \
                 -L../cass_ccd -lcass_ccd \
                 -L../cass_machinedata -lcass_machinedata \
                 -L../LCLS/build/pdsdata/lib/x86_64-linux \
                 -lacqdata -lxtcdata -lpulnixdata -lcamdata -lpnccddata \
                 -levrdata -lappdata \
                 -lgsoap++ -lgsoap

TARGETDEPS    += ../cass_acqiris/libcass_acqiris.a \
                 ../cass_pnccd/libcass_pnccd.a \
                 ../cass_ccd/libcass_ccd.a \
                 ../cass_machinedata/libcass_machinedata.a

bin.path       = $$INSTALLBASE/bin
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
bin.files      = cass
header.files   = $$HEADERS
libs.files     = libcass.a

QMAKE_CLEAN   += cass
INSTALLS      += header libs bin




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009,2010 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE       = app
TARGET         = cass
CONFIG        += release
CONFIG        += thread warn_on exceptions rtti sse2 stl
CONFIG        += static staticlib

CODECFORTR     = UTF-8
DEFINES       += CASS_LIBRARY
MOC_DIR        = ./obj
OBJECTS_DIR    = ./obj
QMAKE_STRIP    =
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET
VERSION        = 0.1.0

# build the LCLS libraries and programs before compiling cass itself
lclslibs.target     = LCLSLibrary
lclslibs.commands   = @cd $$PWD/../LCLS && make x86_64-linux-static-opt
lclslibs.depends    = FORCE
lclslibs.path       = $$INSTALLBASE/lib
lclslibs.files      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libacqdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libappdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libbld.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcamdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcontroldata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libepics.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libevrdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libipimbdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libopal1kdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpnccddata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libprincetondata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpulnixdata.a \
                      $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libxtcdata.a

lclsapps.target     = LCLSApplication
lclsapps.commands   = @cd $$PWD/../LCLS && make x86_64-linux-static-opt
lclsapps.depends    = FORCE
lclsapps.path       = $$INSTALLBASE/bin
lclsapps.files      = $$PWD/../LCLS/build/pdsdata/bin/x86_64-linux-static-opt/xtcmonserver

INSTALLS           += lclsapps
QMAKE_CLEAN        += $$lclslibs.files $$lclsapps.files \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/ConfigV3.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/PulseConfigV3.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/OutputMap.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/PulseConfig.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/EventCodeV3.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/ConfigV2.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/evr/src/DataV3.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/epics/src/EpicsPvData.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/epics/src/EpicsXtcSettings.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/epics/src/EpicsDbrTools.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/app/xtcmonserver.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/app/XtcMonitorClient.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/opal1k/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/pnCCD/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/pnCCD/src/FrameV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/ipimb/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/ipimb/src/DataV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/acqiris/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/acqiris/src/DataDescV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/control/src/PVControl.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/control/src/PVMonitor.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/control/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/camera/src/FrameFexConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/camera/src/TwoDGaussianV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/camera/src/FrameV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/XtcIterator.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/TransitionId.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/BldInfo.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/DetInfo.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/XtcFileIterator.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/Src.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/Level.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/TypeId.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/ClockTime.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/TimeStamp.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/ProcInfo.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/xtc/src/Sequence.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/bld/src/bldData.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/princeton/src/ConfigV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/princeton/src/FrameV1.o \
                      $$PWD/../LCLS/build/pdsdata/obj/x86_64-linux-static-opt/pulnix/src/TM6740ConfigV1.o

# create SOAP sources and descriptions
# use "newer" to make sure soapcpp2 is only run when necessary (this should be done by qmake, though!)
SOAPFiles.target    = CASSsoapService
SOAPFiles.commands  = newer soapCASSsoapService.h $$PWD/soapserver.h || soapcpp2 -S -i $$PWD/soapserver.h
SOAPFiles.files    += soapCASSsoapService.cpp soapCASSsoapService.h soapC.cpp soapH.h soapStub.h \
	              CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
		      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
		      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
QMAKE_CLEAN        += $$SOAPFiles.files

PRE_TARGETDEPS     += CASSsoapService LCLSLibrary LCLSApplication
QMAKE_EXTRA_TARGETS+= SOAPFiles lclslibs lclsapps

# our own stuff
SOURCES +=  analyzer.cpp \
            cass.cpp \
            cass_event.cpp \
            daemon.cpp \
            event_getter.cpp \
            format_converter.cpp \
            histogram.cpp \
            histogram_getter.cpp \
            sharedmemory_input.cpp \
            ratemeter.cpp \
            worker.cpp \
            rate_plotter.cpp \
            ./postprocessing/postprocessor.cpp \
            ./postprocessing/ccd.cpp \
            ./postprocessing/alignment.cpp \
            ./postprocessing/imaging.cpp \
	    soapCASSsoapService.cpp \
            soapC.cpp \
	    tcpserver.cpp
            # ./postprocessing/waveform.cpp \
            # ./postprocessing/acqiris_detectors.cpp \

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
            ./postprocessing/imaging.h \
            soapCASSsoapService.h \
            soapH.h \
            soapStub.h \
            tcpserver.h
	    # ./postprocessing/waveform.h \
            # ./postprocessing/acqiris_detectors.h \

INCLUDEPATH   += postprocessing \
                 ../cass_acqiris \
                 ../cass_acqiris/classes \
                 ../cass_acqiris/classes/detector_analyzer \
                 ../cass_acqiris/classes/waveformanalyzer \
                 ../cass_ccd \
                 ../cass_pnccd \
                 ../cass_machinedata \
                 $$PWD/../LCLS \
                 .

LIBS          += -L../cass_acqiris -lcass_acqiris \
                 -L../cass_pnccd -lcass_pnccd \
                 -L../cass_ccd -lcass_ccd \
                 -L../cass_machinedata -lcass_machinedata \
                 -L$$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt \
                 -lappdata -lacqdata -lcamdata -levrdata -lpnccddata -lpulnixdata -lxtcdata \
                 -lgsoap++ -lgsoap

TARGETDEPS    += ../cass_acqiris/libcass_acqiris.a \
                 ../cass_pnccd/libcass_pnccd.a \
                 ../cass_ccd/libcass_ccd.a \
                 ../cass_machinedata/libcass_machinedata.a

bin.path       = $$INSTALLBASE/bin
bin.files      = cass
header.path    = $$INSTALLBASE/include
header.files   = $$HEADERS
libs.path      = $$INSTALLBASE/lib
libs.files     =

INSTALLS      += header libs bin




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

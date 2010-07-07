# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009, 2010 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET               = cass
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $$INSTALLBASE/bin
CONFIG              -= gui
DEFINES             += CASS_LIBRARY


# build the LCLS libraries and programs before compiling cass itself by adding it to the pre traget dependencies
PRE_TARGETDEPS     += $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libacqdata.a \
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

lclsacq.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libacqdata.a
lclsacq.commands    = @cd $$PWD/../LCLS && make x86_64-linux-static-opt
lclsacq.depends     = FORCE

lclsapp.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libappdata.a
lclsapp.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libacqdata.a

lclsbld.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libbld.a
lclsbld.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libappdata.a

lclscam.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcamdata.a
lclscam.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libbld.a

lclsctrl.target     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcontroldata.a
lclsctrl.depends    = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcamdata.a

lclsepic.target     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libepics.a
lclsepic.depends    = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libcontroldata.a

lclsevr.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libevrdata.a
lclsevr.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libepics.a

lclsipm.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libipimbdata.a
lclsipm.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libevrdata.a

lclsopal.target     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libopal1kdata.a
lclsopal.depends    = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libipimbdata.a

lclspnccd.target    = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpnccddata.a
lclspnccd.depends   = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libopal1kdata.a

lclsprinc.target    = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libprincetondata.a
lclsprinc.depends   = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpnccddata.a

lclspul.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpulnixdata.a
lclspul.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libprincetondata.a

lclsxtc.target      = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libxtcdata.a
lclsxtc.depends     = $$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt/libpulnixdata.a

QMAKE_EXTRA_TARGETS+= lclsacq lclsapp lclsbld lclscam lclsctrl lclsepic lclsevr lclsipm lclsopal lclspnccd lclsprinc lclspul lclsxtc


# create SOAP sources and descriptions
SOAPFiles.target    = soapCASSsoapService.cpp
SOAPFiles.commands  = @soapcpp2 -S -i $$PWD/soapserver.h
SOAPFiles.files    += soapCASSsoapService.cpp soapCASSsoapService.h soapC.cpp soapH.h soapStub.h \
                      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
                      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
                      CASSsoap.clearHistogram.req.xml CASSsoap.clearHistogram.res.xml \
                      CASSsoap.getPostprocessorIds.req.xml CASSsoap.getPostprocessorIds.res.xml \
                      CASSsoap.writeini.req.xml CASSsoap.writeini.res.xml \
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
SOAPFiles.depends   = soapserver.h

SOAPFiles2.target   = soapC.cpp
SOAPFiles2.depends  = soapCASSsoapService.cpp

QMAKE_EXTRA_TARGETS+= SOAPFiles SOAPFiles2

# our own stuff
SOURCES            += soapCASSsoapService.cpp \
                      soapC.cpp \
                      analyzer.cpp \
                      cass.cpp \
                      cass_event.cpp \
                      daemon.cpp \
                      event_getter.cpp \
                      file_input.cpp \
                      format_converter.cpp \
                      histogram.cpp \
                      histogram_getter.cpp \
                      sharedmemory_input.cpp \
                      ratemeter.cpp \
                      worker.cpp \
                      pixel_detector.cpp \
                      rate_plotter.cpp \
                      cass_settings.cpp \
                      ./postprocessing/convenience_functions.cpp \
                      ./postprocessing/backend.cpp \
                      ./postprocessing/waveform.cpp \
                      ./postprocessing/acqiris_detectors_helper.cpp \
                      ./postprocessing/acqiris_detectors.cpp \
                      ./postprocessing/machine_data.cpp \
                      ./postprocessing/ccd.cpp \
                      ./postprocessing/alignment.cpp \
                      ./postprocessing/imaging.cpp \
                      ./postprocessing/postprocessor.cpp \
                      ./postprocessing/id_list.cpp \
                      ./postprocessing/operations.cpp \
                      tcpserver.cpp \

HEADERS            += analysis_backend.h \
                      analyzer.h \
                      cass.h \
                      cass_event.h \
                      ccd_detector.h \
                      conversion_backend.h \
                      daemon.h \
                      event_getter.h \
                      file_input.h \
                      format_converter.h \
                      histogram.h \
                      histogram_getter.h \
                      ratemeter.h \
                      ringbuffer.h \
                      rate_plotter.h \
                      serializable.h \
                      serializer.h \
                      sharedmemory_input.h \
                      soapCASSsoapService.h \
                      soapH.h \
                      soapStub.h \
                      tcpserver.h \
                      worker.h \
                      pixel_detector.h \
                      xtciterator.h \
                      cass_exceptions.h \
                      cass_settings.h \
                      ./postprocessing/convenience_functions.h \
                      ./postprocessing/postprocessor.h \
                      ./postprocessing/id_list.h \
                      ./postprocessing/acqiris_detectors.h \
                      ./postprocessing/acqiris_detectors_helper.h \
                      ./postprocessing/operations.h \
                      ./postprocessing/operation_templates.hpp \
                      ./postprocessing/alignment.h \
                      ./postprocessing/backend.h \
                      ./postprocessing/ccd.h \
                      ./postprocessing/imaging.h \
                      ./postprocessing/waveform.h \
                      ./postprocessing/machine_data.h \
                      ./postprocessing/root_converter.h \
                      ./postprocessing/hdf5dump.h \
                      ./postprocessing/hdf5_converter.h \

INCLUDEPATH        += postprocessing \
                      ../cass_acqiris \
                      ../cass_acqiris/classes \
                      ../cass_acqiris/classes/detector_analyzer \
                      ../cass_acqiris/classes/waveformanalyzer \
                      ../cass_ccd \
                      ../cass_pnccd \
                      ../cass_machinedata \
                      $$PWD/../LCLS

DEPENDPATH         += ./postprocessing

LIBS               += -L$${CASS_ROOT}/lib -lcass_acqiris -lcass_pnccd -lcass_ccd -lcass_machinedata \
                      -L$$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt \
                      -lappdata -lacqdata -lcamdata -levrdata -lpnccddata -lpulnixdata -lxtcdata \
                      -lgsoap++ -lgsoap

# Extra stuff if compiling pp1000,pp1001
hdf5 {
    INCLUDEPATH    += $$(HDF5DIR)/include
    LIBS           += -L$$(HDF5DIR)/lib -lhdf5
    SOURCES        += ./postprocessing/hdf5dump.cpp \
                      ./postprocessing/hdf5_converter.cpp
    DEFINES        += HDF5
}

# extra files if compiling single particle detector.
# depends on VIGRA template library. (by Ullrich Koethe)
singleparticle_hit {
    SOURCES        += ./postprocessing/hitrate.cpp
    HEADERS        += ./postprocessing/hitrate.h
    DEFINES        += SINGLEPARTICLE_HIT
}

cernroot {
    INCLUDEPATH    += $$(ROOTSYS)/include
    LIBS           += -L$$(ROOTSYS)/lib -lHist -lRIO -lCore -lMathCore -lMatrix -lCint
    SOURCES        += ./postprocessing/root_converter.cpp
    DEFINES        += CERNROOT
}

CONFIG(offline) {
    DEFINES        += OFFLINE RINGBUFFER_BLOCKING
}

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
lclsapps. path      = $$INSTALLBASE/bin
lclsapps.files      = $$PWD/../LCLS/build/pdsdata/bin/x86_64-linux-static-opt/xtcmonserver
bin_copy.path       = ./
bin_copy.extra     += bash backup_copy.sh $${INSTALLBASE}
headers.files       = $$HEADERS
INSTALLS           += target bin_copy lclslibs lclsapps

#INSTALLS           += headers target bin_copy lclslibs lclsapps


versiontarget.target = $$PWD/../cass/update-version.sh
versiontarget.commands = $$PWD/../cass/update-version.sh
versiontarget.depends= FORCE

#PRE_TARGETDEPS     += $$PWD/../cass/update-version.sh
#QMAKE_EXTRA_TARGETS+= versiontarget

QMAKE_CLEAN        += $$SOAPFiles.files
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


## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

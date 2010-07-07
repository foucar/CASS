# Copyright (C) 2010 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET         = cass
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $$INSTALLBASE/lib

CONFIG        -= gui

DEFINES       += CASS_LIBRARY

SOURCES       += analyzer.cpp \
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
                 soapCASSsoapService.cpp \
                 soapC.cpp \
                 tcpserver.cpp \

HEADERS       += analysis_backend.h \
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

INCLUDEPATH   += postprocessing \
                 ../cass_acqiris \
                 ../cass_acqiris/classes \
                 ../cass_acqiris/classes/detector_analyzer \
                 ../cass_acqiris/classes/waveformanalyzer \
                 ../cass_ccd \
                 ../cass_pnccd \
                 ../cass_machinedata \
                 $$PWD/../LCLS

DEPENDPATH    += ./postprocessing

# Extra stuff if compiling pp1000,pp1001
hdf5 {
    INCLUDEPATH += $$(HDF5DIR)/include
    SOURCES += ./postprocessing/hdf5dump.cpp \
               ./postprocessing/hdf5_converter.cpp
    DEFINES += HDF5
}

# extra files if compiling single particle detector.
# depends on VIGRA template library. (by Ullrich Koethe)
singleparticle_hit {
    SOURCES +=  ./postprocessing/hitrate.cpp
    HEADERS +=  ./postprocessing/hitrate.h
    DEFINES += SINGLEPARTICLE_HIT
}

cernroot {
    INCLUDEPATH += $$(ROOTSYS)/include
    SOURCES += ./postprocessing/root_converter.cpp
    DEFINES += CERNROOT
}


CONFIG(offline) {
    DEFINES += OFFLINE RINGBUFFER_BLOCKING
}

headers.files  = $$HEADERS
INSTALLS      += target
#INSTALLS      += headers target bin_copy

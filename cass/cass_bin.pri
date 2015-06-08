# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 - 2015 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $${PREFIX}/bin
CONFIG             -= gui
QT                 += network
DEFINES            += CASS_LIBRARY

# create SOAP sources and descriptions
SOAP_INPUTFILE      = $$PWD/soapserver.h
SOAP_OUTPUTFILE     = soapCASSsoapService.cpp
SOAP_BIN            = $$GSOAP_BIN -S
include( $$PWD/soapfile_generator.pri )

SOURCES            += \
                      cass.cpp \
                      cass_event.cpp \
                      conversion_backend.cpp \
                      log.cpp \
                      input_base.cpp \
                      file_input.cpp \
                      file_reader.cpp \
                      file_parser.cpp \
                      xtc_reader.cpp \
                      xtc_parser.cpp \
                      txt_reader.cpp \
                      txt_parser.cpp \
                      multifile_input.cpp \
                      pausablethread.cpp \
                      format_converter.cpp \
                      histogram.cpp \
                      sharedmemory_input.cpp \
                      tcp_input.cpp \
                      tcp_streamer.cpp \
#                      test_input.cpp \
#                      data_generator.cpp \
#                      waveform_generator.cpp \
#                      image_generator.cpp \
                      ratemeter.cpp \
                      worker.cpp \
                      rate_plotter.cpp \
                      cass_settings.cpp \
                      calibcycle.cpp \
                      tcpserver.cpp \
                      geom_parser.cpp \
                      ./postprocessing/processor.cpp \
                      ./postprocessing/processor_manager.cpp \
                      ./postprocessing/convenience_functions.cpp \
                      ./postprocessing/operations.cpp \
                      ./postprocessing/waveform.cpp \
                      ./postprocessing/acqiris_detectors_helper.cpp \
                      ./postprocessing/acqiris_detectors.cpp \
                      ./postprocessing/machine_data.cpp \
                      ./postprocessing/pixel_detector_helper.cpp \
                      ./postprocessing/pixel_detectors.cpp \
                      ./postprocessing/alignment.cpp \
                      ./postprocessing/imaging.cpp \
                      ./postprocessing/id_list.cpp \
                      ./postprocessing/rankfilter.cpp \
                      ./postprocessing/coltrims_analysis.cpp \
                      ./postprocessing/achimcalibrator_hex.cpp \
                      ./postprocessing/image_manipulation.cpp \
                      ./postprocessing/hitfinder.cpp \
                      ./postprocessing/partial_covariance.cpp \
                      ./postprocessing/cbf_output.cpp \
                      ./postprocessing/table_operations.cpp \
                      ./postprocessing/autocorrelation.cpp \
                      ./postprocessing/pixel_detector_calibration.cpp


HEADERS            += cass.h \
                      cl_parser.hpp \
                      hdf5_handle.hpp \
                      cass_event.h \
                      conversion_backend.h \
                      log.h \
                      input_base.h \
                      file_input.h \
                      file_parser.h \
                      file_reader.h \
                      xtc_reader.h \
                      xtc_parser.h \
                      hlltypes.h \
                      raw_sss_file_header.h \
                      txt_reader.h \
                      txt_parser.h \
                      multifile_input.h \
                      pausablethread.h \
                      format_converter.h \
                      histogram.h \
                      ratemeter.h \
                      ringbuffer.h \
                      rate_plotter.h \
                      serializable.h \
                      serializer.h \
                      sharedmemory_input.h \
                      tcp_input.h \
                      tcp_streamer.h \
#                      test_input.h \
#                      data_generator.h \
#                      waveform_generator.h \
#                      image_generator.h \
                      tcpserver.h \
                      worker.h \
                      xtciterator.hpp \
                      cass_exceptions.h \
                      cass_settings.h \
                      calibcycle.h \
                      statistics_calculator.hpp \
                      generic_factory.hpp \
                      cached_list.hpp \
                      geom_parser.h \
                      ./postprocessing/processor.h \
                      ./postprocessing/processor_manager.h \
                      ./postprocessing/convenience_functions.h \
                      ./postprocessing/id_list.h \
                      ./postprocessing/acqiris_detectors.h \
                      ./postprocessing/acqiris_detectors_helper.h \
                      ./postprocessing/operations.h \
                      ./postprocessing/rankfilter.h \
                      ./postprocessing/alignment.h \
                      ./postprocessing/pixel_detector_helper.h \
                      ./postprocessing/pixel_detectors.h \
                      ./postprocessing/imaging.h \
                      ./postprocessing/waveform.h \
                      ./postprocessing/machine_data.h \
                      ./postprocessing/root_converter.h \
                      ./postprocessing/rootfile_helper.h \
                      ./postprocessing/roottree_converter.h \
                      ./postprocessing/tree_structure.h \
                      ./postprocessing/hdf5_converter.h \
                      ./postprocessing/coltrims_analysis.h \
                      ./postprocessing/achimcalibrator_hex.h \
                      ./postprocessing/image_manipulation.h \
                      ./postprocessing/hitfinder.h \
                      ./postprocessing/partial_covariance.h \
                      ./postprocessing/cbf_output.h \
                      ./postprocessing/table_operations.h \
                      ./postprocessing/autocorrelation.h \
                      ./postprocessing/pixel_detector_calibration.h

INCLUDEPATH        += postprocessing \
                      ../cass_acqiris \
                      ../cass_acqiris/classes \
                      ../cass_acqiris/classes/detector_analyzer \
                      ../cass_acqiris/classes/signalextractors \
                      ../cass_acqiris/classes/momenta_calculators \
                      ../cass_ccd \
                      ../cass_pnccd \
                      ../cass_machinedata \
                      ../cass_pixeldetector \
                      $$PWD/../LCLS

DEPENDPATH         += ./postprocessing

DEPENDENCY_LIBRARIES  = cass_acqiris \
                        cass_machinedata \
                        cass_pixeldetector

DEPENDENCY_LIBRARIES += acqdata \
                        appdata \
                        bld \
                        camdata \
                        compressdata \
                        controldata \
                        cspaddata \
                        cspad2x2data \
                        encoderdata \
                        epics \
                        evrdata \
                        fccddata \
                        ipimbdata \
                        lusidata \
                        opal1kdata \
                        pnccddata \
                        princetondata \
                        pulnixdata \
                        xtcdata \

include( $${CASS_ROOT}/cass_dependencies.pri )

LIBS               += -lgsoap++ -lgsoap
LIBS               += -L$${CASS_ROOT}/cass_acqiris/classes/detector_analyzer/resorter -lResort64c_x64
LIBS               += -lrt


# Extra stuff for http Server
httpServer {
    LIBS           += -lmicrohttpd
    LIBS           += -ljpeg
    SOURCES        += ./httpserver.cpp
    HEADERS        += ./httpserver.h
    DEFINES        += HTTPSERVER
    DEFINES        += JPEG_CONVERSION
}

# Extra stuff if compiling postprocessors with hdf5 support
hdf5 {
    LIBS           += -lhdf5
    SOURCES        += ./postprocessing/hdf5_converter.cpp
    DEFINES        += HDF5
}

# extra files if compiling single particle detector.
# depends on VIGRA template library. (by Ullrich Koethe)
singleparticle_hit {
    SOURCES        += ./postprocessing/hitrate.cpp
    HEADERS        += ./postprocessing/hitrate.h
    DEFINES        += SINGLEPARTICLE_HIT
}

#extra stuff for ROOT
cernroot {
    LIBS           += $$system($$ROOTCONFIG_BIN --libs)
    SOURCES        += ./postprocessing/root_converter.cpp
    SOURCES        += ./postprocessing/rootfile_helper.cpp
    SOURCES        += ./postprocessing/roottree_converter.cpp
    DEFINES        += CERNROOT
    DICTIONARYFILES = ./postprocessing/tree_structure.h
    include( $$PWD/rootdict_generator.pri )
}

# Extra stuff for fftw
fftw {
    LIBS           += -lfftw3
    SOURCES        += ./postprocessing/fft.cpp
    HEADERS        += ./postprocessing/fft.h
    DEFINES        += FFTW
}

# Extra stuff for SACLA DATA
SACLA {
    LIBS           += $$SACLA_OFFLINE_LIBDIR/libSaclaDataAccessUserAPI.a
    LIBS           += -lmysqlclient
#    LIBS           += -lirc
#    LIBS           += -limf
    SOURCES        += ./sacla_offline_input.cpp
    HEADERS        += ./sacla_offline_input.h
    SOURCES        += ./sacla_converter.cpp
    HEADERS        += ./sacla_converter.h
    DEFINES        += SACLADATA
    online {
           LIBS    += $$SACLA_ONLINE_LIBDIR/libOnlineUserAPI.a
           SOURCES += ./sacla_online_input.cpp
           HEADERS += ./sacla_online_input.h
    }
}

INSTALLS           += target

# execute script that shows the current version derived from git
#version.target      = cass_version.h
version.commands    = $$PWD/update-version.sh
QMAKE_EXTRA_TARGETS+= version
PRE_TARGETDEPS     += version

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

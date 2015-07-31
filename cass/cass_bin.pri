# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 - 2015 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $${PREFIX}/bin
QT                 -= gui
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
                      pausablethread.cpp \
                      format_converter.cpp \
                      histogram.cpp \
#                      test_input.cpp \
#                      data_generator.cpp \
#                      waveform_generator.cpp \
#                      image_generator.cpp \
                      ratemeter.cpp \
                      worker.cpp \
                      rate_plotter.cpp \
                      cass_settings.cpp \
                      tcpserver.cpp \
                      geom_parser.cpp \
                      ./processing/processor.cpp \
                      ./processing/processor_manager.cpp \
                      ./processing/convenience_functions.cpp \
                      ./processing/operations.cpp \
                      ./processing/waveform.cpp \
                      ./processing/acqiris_detectors_helper.cpp \
                      ./processing/acqiris_detectors.cpp \
                      ./processing/machine_data.cpp \
                      ./processing/pixel_detector_helper.cpp \
                      ./processing/pixel_detectors.cpp \
                      ./processing/alignment.cpp \
                      ./processing/imaging.cpp \
                      ./processing/id_list.cpp \
                      ./processing/rankfilter.cpp \
                      ./processing/coltrims_analysis.cpp \
                      ./processing/achimcalibrator_hex.cpp \
                      ./processing/image_manipulation.cpp \
                      ./processing/hitfinder.cpp \
                      ./processing/partial_covariance.cpp \
                      ./processing/cbf_output.cpp \
                      ./processing/table_operations.cpp \
                      ./processing/autocorrelation.cpp \
                      ./processing/pixel_detector_calibration.cpp


HEADERS            += cass.h \
                      cl_parser.hpp \
                      cass_event.h \
                      conversion_backend.h \
                      log.h \
                      input_base.h \
                      hlltypes.h \
                      raw_sss_file_header.h \
                      pausablethread.h \
                      format_converter.h \
                      histogram.h \
                      ratemeter.h \
                      ringbuffer.h \
                      rate_plotter.h \
                      serializable.h \
                      serializer.h \
#                      test_input.h \
#                      data_generator.h \
#                      waveform_generator.h \
#                      image_generator.h \
                      tcpserver.h \
                      worker.h \
                      xtciterator.hpp \
                      cass_exceptions.h \
                      cass_settings.h \
                      statistics_calculator.hpp \
#                      generic_factory.hpp \
                      cached_list.hpp \
                      geom_parser.h \
                      ./processing/processor.h \
                      ./processing/processor_manager.h \
                      ./processing/convenience_functions.h \
                      ./processing/id_list.h \
                      ./processing/acqiris_detectors.h \
                      ./processing/acqiris_detectors_helper.h \
                      ./processing/operations.h \
                      ./processing/rankfilter.h \
                      ./processing/alignment.h \
                      ./processing/pixel_detector_helper.h \
                      ./processing/pixel_detectors.h \
                      ./processing/imaging.h \
                      ./processing/waveform.h \
                      ./processing/machine_data.h \
                      ./processing/root_converter.h \
                      ./processing/rootfile_helper.h \
                      ./processing/roottree_converter.h \
                      ./processing/tree_structure.h \
                      ./processing/coltrims_analysis.h \
                      ./processing/achimcalibrator_hex.h \
                      ./processing/image_manipulation.h \
                      ./processing/hitfinder.h \
                      ./processing/partial_covariance.h \
                      ./processing/cbf_output.h \
                      ./processing/table_operations.h \
                      ./processing/autocorrelation.h \
                      ./processing/pixel_detector_calibration.h

INCLUDEPATH        += processing \
                      ../cass_acqiris \
                      ../cass_acqiris/classes \
                      ../cass_acqiris/classes/detector_analyzer \
                      ../cass_acqiris/classes/signalextractors \
                      ../cass_acqiris/classes/momenta_calculators \
                      ../cass_machinedata \
                      ../cass_pixeldetector \
                      $$PWD/../LCLS

DEPENDPATH         += ./processing

# the dependencies of other libraries of cass
DEPENDENCY_LIBRARIES  = cass_acqiris \
                        cass_machinedata \
                        cass_pixeldetector

# the dependencies of the lcls libraries
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


# Stuff needed offline for offline version
is_offline {
    SOURCES        += file_input.cpp
    HEADERS        += file_input.h
    SOURCES        += multifile_input.cpp
    HEADERS        += multifile_input.h
    SOURCES        += file_reader.cpp
    HEADERS        += file_reader.h
    SOURCES        += file_parser.cpp
    HEADERS        += file_parser.h
    SOURCES        += xtc_reader.cpp
    HEADERS        += xtc_reader.h
    SOURCES        += xtc_parser.cpp
    HEADERS        += xtc_parser.h
    SOURCES        += txt_reader.cpp
    HEADERS        += txt_reader.h
    SOURCES        += txt_parser.cpp
    HEADERS        += txt_parser.h
}

# Stuff needed online for online version
is_online {
    SOURCES        += sharedmemory_input.cpp
    HEADERS        += sharedmemory_input.h
    SOURCES        += tcp_input.cpp
    HEADERS        += tcp_input.h
    SOURCES        += tcp_streamer.cpp
    HEADERS        += tcp_streamer.h
}

# Extra stuff for http Server
httpServer {
    LIBS           += -lmicrohttpd
    LIBS           += -ljpeg
    SOURCES        += ./httpserver.cpp
    HEADERS        += ./httpserver.h
    DEFINES        += HTTPSERVER
    DEFINES        += JPEG_CONVERSION
}

# Extra stuff if compiling processors with hdf5 support
hdf5 {
    LIBS           += -lhdf5
    SOURCES        += ./processing/hdf5_converter.cpp
    HEADERS        += ./processing/hdf5_converter.h
    HEADERS        += hdf5_handle.hpp
    DEFINES        += HDF5
    is_offline {
        HEADERS    += hdf5_file_input.h
        SOURCES    += hdf5_file_input.cpp
    }
}

# extra files if compiling single particle detector.
# depends on VIGRA template library. (by Ullrich Koethe)
singleparticle_hit {
    SOURCES        += ./processing/hitrate.cpp
    HEADERS        += ./processing/hitrate.h
    DEFINES        += SINGLEPARTICLE_HIT
}

#extra stuff for ROOT
cernroot {
    SOURCES        += ./processing/root_converter.cpp
    HEADERS        += ./processing/root_converter.h
    SOURCES        += ./processing/rootfile_helper.cpp
    HEADERS        += ./processing/rootfile_helper.h
    SOURCES        += ./processing/roottree_converter.cpp
    HEADERS        += ./processing/roottree_converter.h
    HEADERS        += ./processing/tree_structure.h
    HEADERS        += ./processing/tree_structure_linkdef.h
    LIBS           += $$system($$ROOTCONFIG_BIN --libs)
    DEFINES        += CERNROOT
    DICTIONARYFILES = ./processing/tree_structure.h
    include( $$PWD/rootdict_generator.pri )
}

# Extra stuff for fftw
fftw {
    SOURCES        += ./processing/fft.cpp
    HEADERS        += ./processing/fft.h
    LIBS           += -lfftw3
    DEFINES        += FFTW
}

# Extra stuff for SACLA DATA
SACLA {
    is_offline {
        SOURCES    += ./sacla_offline_input.cpp
        HEADERS    += ./sacla_offline_input.h
        SOURCES    += ./sacla_converter.cpp
        HEADERS    += ./sacla_converter.h
    }
    is_online {
        LIBS       += $$SACLA_ONLINE_LIBDIR/libOnlineUserAPI.a
        SOURCES    += ./sacla_online_input.cpp
        HEADERS    += ./sacla_online_input.h
    }
    LIBS           += $$SACLA_OFFLINE_LIBDIR/libSaclaDataAccessUserAPI.a
    LIBS           += -lmysqlclient
#    LIBS           += -lirc
#    LIBS           += -limf
    DEFINES        += SACLADATA
}

# other libraries needed
LIBS               += -lgsoap++ -lgsoap
LIBS               += -L$${CASS_ROOT}/cass_acqiris/classes/detector_analyzer/resorter -lResort64c_x64
LIBS               += -lrt


INSTALLS           += target

# execute script that shows the current version derived from git
#version.target      = cass_version.h
version.commands    = $$PWD/update-version.sh
QMAKE_EXTRA_TARGETS+= version
PRE_TARGETDEPS     += version

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 - 2015 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

TEMPLATE                = app
DESTDIR                 = $${CASS_ROOT}/bin
target.path             = $${PREFIX}/bin
QT                     -= gui
QT                     += network

# create SOAP sources and descriptions
SOAP_INPUTFILE          = soapserver.h
SOAP_OUTPUTFILE         = soapCASSsoapService.cpp
SOAP_BIN                = $$GSOAP_BIN -S
include( $$PWD/soapfile_generator.pri )

SOURCES                += cass.cpp
HEADERS                += cass.h
SOURCES                += cass_event.cpp
HEADERS                += cass_event.h
SOURCES                += conversion_backend.cpp
HEADERS                += conversion_backend.h
SOURCES                += log.cpp
HEADERS                += log.h
SOURCES                += input_base.cpp
HEADERS                += input_base.h
SOURCES                += pausablethread.cpp
HEADERS                += pausablethread.h
SOURCES                += format_converter.cpp
HEADERS                += format_converter.h
SOURCES                += histogram.cpp
HEADERS                += histogram.h
SOURCES                += ratemeter.cpp
HEADERS                += ratemeter.h
SOURCES                += worker.cpp
HEADERS                += worker.h
SOURCES                += rate_plotter.cpp
HEADERS                += rate_plotter.h
SOURCES                += cass_settings.cpp
HEADERS                += cass_settings.h
SOURCES                += tcpserver.cpp
HEADERS                += tcpserver.h
SOURCES                += geom_parser.cpp
HEADERS                += geom_parser.h
SOURCES                += ./processing/processor.cpp
HEADERS                += ./processing/processor.h
SOURCES                += ./processing/processor_manager.cpp
HEADERS                += ./processing/processor_manager.h
SOURCES                += ./processing/convenience_functions.cpp
HEADERS                += ./processing/convenience_functions.h
SOURCES                += ./processing/operations.cpp
HEADERS                += ./processing/operations.h
SOURCES                += ./processing/waveform.cpp
HEADERS                += ./processing/waveform.h
SOURCES                += ./processing/acqiris_detectors_helper.cpp
HEADERS                += ./processing/acqiris_detectors_helper.h
SOURCES                += ./processing/acqiris_detectors.cpp
HEADERS                += ./processing/acqiris_detectors.h
SOURCES                += ./processing/machine_data.cpp
HEADERS                += ./processing/machine_data.h
SOURCES                += ./processing/pixel_detector_helper.cpp
HEADERS                += ./processing/pixel_detector_helper.h
SOURCES                += ./processing/pixel_detectors.cpp
HEADERS                += ./processing/pixel_detectors.h
SOURCES                += ./processing/alignment.cpp
HEADERS                += ./processing/alignment.h
SOURCES                += ./processing/imaging.cpp
HEADERS                += ./processing/imaging.h
SOURCES                += ./processing/id_list.cpp
HEADERS                += ./processing/id_list.h
SOURCES                += ./processing/rankfilter.cpp
HEADERS                += ./processing/rankfilter.h
SOURCES                += ./processing/coltrims_analysis.cpp
HEADERS                += ./processing/coltrims_analysis.h
SOURCES                += ./processing/achimcalibrator_hex.cpp
HEADERS                += ./processing/achimcalibrator_hex.h
SOURCES                += ./processing/image_manipulation.cpp
HEADERS                += ./processing/image_manipulation.h
SOURCES                += ./processing/hitfinder.cpp
HEADERS                += ./processing/hitfinder.h
SOURCES                += ./processing/partial_covariance.cpp
HEADERS                += ./processing/partial_covariance.h
SOURCES                += ./processing/cbf_output.cpp
HEADERS                += ./processing/cbf_output.h
SOURCES                += ./processing/table_operations.cpp
HEADERS                += ./processing/table_operations.h
SOURCES                += ./processing/autocorrelation.cpp
HEADERS                += ./processing/autocorrelation.h
SOURCES                += ./processing/pixel_detector_calibration.cpp
HEADERS                += ./processing/pixel_detector_calibration.h

HEADERS                += cl_parser.hpp
HEADERS                += hlltypes.h
HEADERS                += raw_sss_file_header.h
HEADERS                += ringbuffer.h
HEADERS                += serializable.h
HEADERS                += serializer.h
HEADERS                += xtciterator.hpp
HEADERS                += cass_exceptions.h
HEADERS                += statistics_calculator.hpp
HEADERS                += cached_list.hpp

INCLUDEPATH            += processing
INCLUDEPATH            += $${CASS_ROOT}/cass_acqiris
INCLUDEPATH            += $${CASS_ROOT}/cass_acqiris/classes
INCLUDEPATH            += $${CASS_ROOT}/cass_acqiris/classes/detector_analyzer
INCLUDEPATH            += $${CASS_ROOT}/cass_acqiris/classes/signalextractors
INCLUDEPATH            += $${CASS_ROOT}/cass_acqiris/classes/momenta_calculators
INCLUDEPATH            += $${CASS_ROOT}/cass_machinedata
INCLUDEPATH            += $${CASS_ROOT}/cass_pixeldetector
INCLUDEPATH            += $${CASS_ROOT}/LCLS


# the dependencies of other libraries of cass
DEPENDENCY_LIBRARIES   += cass_acqiris
DEPENDENCY_LIBRARIES   += cass_machinedata
DEPENDENCY_LIBRARIES   += cass_pixeldetector

# the dependencies of the lcls libraries
DEPENDENCY_LIBRARIES   += acqdata
DEPENDENCY_LIBRARIES   += appdata
DEPENDENCY_LIBRARIES   += bld
DEPENDENCY_LIBRARIES   += camdata
DEPENDENCY_LIBRARIES   += compressdata
DEPENDENCY_LIBRARIES   += controldata
DEPENDENCY_LIBRARIES   += cspaddata
DEPENDENCY_LIBRARIES   += cspad2x2data
DEPENDENCY_LIBRARIES   += encoderdata
DEPENDENCY_LIBRARIES   += epics
DEPENDENCY_LIBRARIES   += evrdata
DEPENDENCY_LIBRARIES   += fccddata
DEPENDENCY_LIBRARIES   += ipimbdata
DEPENDENCY_LIBRARIES   += lusidata
DEPENDENCY_LIBRARIES   += opal1kdata
DEPENDENCY_LIBRARIES   += pnccddata
DEPENDENCY_LIBRARIES   += princetondata
DEPENDENCY_LIBRARIES   += pulnixdata
DEPENDENCY_LIBRARIES   += xtcdata

include( $${CASS_ROOT}/cass_dependencies.pri )


# Stuff needed offline for offline version
is_offline {
    SOURCES            += file_input.cpp
    HEADERS            += file_input.h
    SOURCES            += multifile_input.cpp
    HEADERS            += multifile_input.h
    SOURCES            += file_reader.cpp
    HEADERS            += file_reader.h
    SOURCES            += file_parser.cpp
    HEADERS            += file_parser.h
    SOURCES            += xtc_reader.cpp
    HEADERS            += xtc_reader.h
    SOURCES            += xtc_parser.cpp
    HEADERS            += xtc_parser.h
    SOURCES            += txt_reader.cpp
    HEADERS            += txt_reader.h
    SOURCES            += txt_parser.cpp
    HEADERS            += txt_parser.h
}

# Stuff needed online for online version
is_online {
    SOURCES            += sharedmemory_input.cpp
    HEADERS            += sharedmemory_input.h
    SOURCES            += tcp_input.cpp
    HEADERS            += tcp_input.h
    SOURCES            += tcp_streamer.cpp
    HEADERS            += tcp_streamer.h
}

# Extra stuff for http Server
httpServer {
    LIBS               += -lmicrohttpd
    LIBS               += -ljpeg
    SOURCES            += ./httpserver.cpp
    HEADERS            += ./httpserver.h
    DEFINES            += HTTPSERVER
    DEFINES            += JPEG_CONVERSION
}

# Extra stuff if compiling processors with hdf5 support
hdf5 {
    LIBS               += -lhdf5
    SOURCES            += ./processing/hdf5_converter.cpp
    HEADERS            += ./processing/hdf5_converter.h
    HEADERS            += hdf5_handle.hpp
    DEFINES            += HDF5
    is_offline {
        HEADERS        += hdf5_file_input.h
        SOURCES        += hdf5_file_input.cpp
    }
}

# extra files if compiling single particle detector.
# depends on VIGRA template library. (by Ullrich Koethe)
singleparticle_hit {
    SOURCES            += ./processing/hitrate.cpp
    HEADERS            += ./processing/hitrate.h
    DEFINES            += SINGLEPARTICLE_HIT
}

#extra stuff for ROOT
cernroot {
    SOURCES            += ./processing/root_converter.cpp
    HEADERS            += ./processing/root_converter.h
    SOURCES            += ./processing/rootfile_helper.cpp
    HEADERS            += ./processing/rootfile_helper.h
    SOURCES            += ./processing/roottree_converter.cpp
    HEADERS            += ./processing/roottree_converter.h
    HEADERS            += ./processing/tree_structure.h
    HEADERS            += ./processing/tree_structure_linkdef.h
    LIBS               += $$system($$ROOTCONFIG_BIN --libs)
    DEFINES            += CERNROOT
    DICTIONARYFILES     = ./processing/tree_structure.h
    include( $$PWD/rootdict_generator.pri )
}

# Extra stuff for fftw
fftw {
    SOURCES            += ./processing/fft.cpp
    HEADERS            += ./processing/fft.h
    LIBS               += -lfftw3
    DEFINES            += FFTW
}

# Extra stuff for SACLA DATA
SACLA {
    is_offline {
        SOURCES        += ./sacla_offline_input.cpp
        HEADERS        += ./sacla_offline_input.h
        SOURCES        += ./sacla_converter.cpp
        HEADERS        += ./sacla_converter.h
    }
    is_online {
        LIBS           += $$SACLA_ONLINE_LIBDIR/libOnlineUserAPI.a
        SOURCES        += ./sacla_online_input.cpp
        HEADERS        += ./sacla_online_input.h
    }
    LIBS               += $$SACLA_OFFLINE_LIBDIR/libSaclaDataAccessUserAPI.a
    LIBS               += -lmysqlclient
#    LIBS               += -lirc
#    LIBS               += -limf
    DEFINES            += SACLADATA
}

# other libraries needed
LIBS                   += -lgsoap++ -lgsoap
LIBS                   += -L$${CASS_ROOT}/cass_acqiris/classes/detector_analyzer/resorter -lResort64c_x64
LIBS                   += -lrt


INSTALLS               += target

# execute script that shows the current version derived from git
#version.target      = cass_version.h
version.commands        = $$PWD/update-version.sh
QMAKE_EXTRA_TARGETS    += version
PRE_TARGETDEPS         += version

QMAKE_CLEAN            += $$OBJECTS_DIR/*.o
QMAKE_CLEAN            += $$MOC_DIR/moc_*
QMAKE_CLEAN            += $$TARGET

#DEPENDPATH         += ./processing
#                          test_input.cpp \
#                          data_generator.cpp \
#                          waveform_generator.cpp \
#                          image_generator.cpp \
#                          test_input.h \
#                          data_generator.h \
#                          waveform_generator.h \
#                          image_generator.h \
#                          generic_factory.hpp \

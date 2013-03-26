# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 - 2013 Lutz Foucar
# Copyright (C) 2009 Nicola Coppola

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

CONFIG(offline) {
  TARGET           = cass_offline
} else {
  TARGET           = cass_online
}

#TARGET               = cass
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $$INSTALLBASE/bin
CONFIG              -= gui
QT                  += network
DEFINES             += CASS_LIBRARY

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
                      conversion_backend.cpp \
                      daemon.cpp \
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
                      ratemeter.cpp \
                      worker.cpp \
                      pixel_detector.cpp \
                      rate_plotter.cpp \
                      cass_settings.cpp \
                      calibcycle.cpp \
                      tcpserver.cpp \
                      statistics_calculator.cpp \
                      ./postprocessing/convenience_functions.cpp \
                      ./postprocessing/backend.cpp \
                      ./postprocessing/waveform.cpp \
                      ./postprocessing/acqiris_detectors_helper.cpp \
                      ./postprocessing/acqiris_detectors.cpp \
                      ./postprocessing/machine_data.cpp \
                      ./postprocessing/ccd.cpp \
                      ./postprocessing/pixel_detector_helper.cpp \
                      ./postprocessing/pixel_detectors.cpp \
                      ./postprocessing/alignment.cpp \
                      ./postprocessing/imaging.cpp \
                      ./postprocessing/operations.cpp \
                      ./postprocessing/postprocessor.cpp \
                      ./postprocessing/id_list.cpp \
                      ./postprocessing/rankfilter.cpp \
                      ./postprocessing/coltrims_analysis.cpp \
                      ./postprocessing/achimcalibrator_hex.cpp \
                      ./postprocessing/image_manipulation.cpp \
                      ./postprocessing/hitfinder.cpp \
                      ./postprocessing/partial_covariance.cpp \
                      ./postprocessing/cbf_output.cpp \
                      ./postprocessing/table_operations.cpp


HEADERS            += analysis_backend.h \
                      analyzer.h \
                      cass.h \
                      cass_event.h \
                      conversion_backend.h \
                      daemon.h \
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
                      soapCASSsoapService.h \
                      soapH.h \
                      soapserver.h \
                      soapStub.h \
                      tcpserver.h \
                      worker.h \
                      xtciterator.h \
                      cass_exceptions.h \
                      cass_settings.h \
                      calibcycle.h \
                      statistics_calculator.h \
                      ./postprocessing/convenience_functions.h \
                      ./postprocessing/postprocessor.h \
                      ./postprocessing/id_list.h \
                      ./postprocessing/acqiris_detectors.h \
                      ./postprocessing/acqiris_detectors_helper.h \
                      ./postprocessing/operations.h \
                      ./postprocessing/rankfilter.h \
                      ./postprocessing/operation_templates.hpp \
                      ./postprocessing/alignment.h \
                      ./postprocessing/backend.h \
                      ./postprocessing/ccd.h \
                      ./postprocessing/pixel_detector_helper.h \
                      ./postprocessing/pixel_detectors.h \
                      ./postprocessing/imaging.h \
                      ./postprocessing/waveform.h \
                      ./postprocessing/machine_data.h \
                      ./postprocessing/root_converter.h \
                      ./postprocessing/rootfile_helper.h \
                      ./postprocessing/roottree_converter.h \
                      ./postprocessing/tree_structure.h \
                      ./postprocessing/hdf5dump.h \
                      ./postprocessing/hdf5_converter.h \
                      ./postprocessing/coltrims_analysis.h \
                      ./postprocessing/achimcalibrator_hex.h \
                      ./postprocessing/image_manipulation.h \
                      ./postprocessing/hitfinder.h \
                      ./postprocessing/partial_covariance.h \
                      ./postprocessing/cbf_output.h \
                      ./postprocessing/table_operations.h

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

DEPENDENCY_LIBRARIES  = cass_acqiris cass_pnccd cass_ccd cass_machinedata cass_pixeldetector
DEPENDENCY_LIBRARIES += appdata acqdata camdata evrdata pnccddata pulnixdata controldata xtcdata ipimbdata lusidata bld cspaddata
include( $${CASS_ROOT}/cass_dependencies.pri )

LIBS               += -lgsoap++ -lgsoap
LIBS               += -L$${CASS_ROOT}/cass_acqiris/classes/detector_analyzer/resorter -lResort64c_x64


# Extra stuff for http Server
httpServer {
    LIBS           += -lmicrohttpd
    LIBS           += -ljpeg
    SOURCES        += ./httpserver.cpp
    HEADERS        += ./httpserver.h
    DEFINES        += HTTPSERVER
    DEFINES        += JPEG_CONVERSION
}

# Extra stuff if compiling pp1000,pp1001
hdf5 {
    INCLUDEPATH    += $$(HDF5DIR)/include
    LIBS           += -L$$(HDF5DIR)/lib -Wl,-rpath=$$(HDF5DIR)/lib -lhdf5
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

#extra stuff for ROOT
cernroot {
    INCLUDEPATH    += $$(ROOTSYS)/include
    LIBS           += $$system(root-config --libs)
    SOURCES        += ./postprocessing/root_converter.cpp
    SOURCES        += ./postprocessing/rootfile_helper.cpp
    DEFINES        += CERNROOT
    SOURCES	       *= ./postprocessing/tree_structure_dict.cpp
    SOURCES        += ./postprocessing/roottree_converter.cpp
    rootcint.target       = ./postprocessing/tree_structure_dict.cpp
    rootcint.commands    += $(ROOTSYS)/bin/rootcint -f $$rootcint.target -c ./postprocessing/tree_structure.h ./postprocessing/tree_structure_linkdef.h
    rootcint.depends      = ./postprocessing/tree_structure.h
    rootcintecho.commands = @echo "Generating dictionary $$rootcint.target for tree_structure.h "
    QMAKE_EXTRA_TARGETS  += rootcintecho rootcint
    QMAKE_CLEAN          +=  ./postprocessing/treestructure_dict.cpp ./postprocessing/treestructure_dict.h
}

CONFIG(offline) {
    DEFINES        += OFFLINE RINGBUFFER_BLOCKING
}

bin_copy.path       = $$INSTALLBASE/bin
bin_copy.extra     += bash backup_copy.sh $${INSTALLBASE} $${TARGET}
headers.files       = $$HEADERS

INSTALLS           += target bin_copy


versiontarget.target = $$PWD/../cass/update-version.sh
versiontarget.commands = $$PWD/../cass/update-version.sh
versiontarget.depends= FORCE

PRE_TARGETDEPS     += $$PWD/../cass/update-version.sh
QMAKE_EXTRA_TARGETS+= versiontarget

QMAKE_CLEAN        += $$SOAPFiles.files
QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

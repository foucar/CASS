# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

CASS_ROOT = ..

include( $${CASS_ROOT}/cass_config.pri )

TARGET = cass_acqiris
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $${PREFIX}/lib

QT -= gui

DEFINES += CASS_ACQIRIS_LIBRARY

INCLUDEPATH += \
    ../cass \
    ../cass/event \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    ./classes/momenta_calculators \
    . \
    ../LCLS

DEPENDPATH += ../cass \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    .
SOURCES += \
    ./acqiris_converter.cpp \
    ./acqiristdc_converter.cpp \
    ./agat_deserializer.cpp \
    ./lma_reader.cpp \
    ./lma_parser.cpp \
    ./classes/particle.cpp \
    ./classes/signal_producer.cpp \
    ./classes/delayline_detector.cpp \
    ./classes/detector_backend.cpp \
    ./classes/tof_detector.cpp \
    ./classes/signalextractors/signal_extractor.cpp \
    ./classes/signalextractors/com.cpp \
    ./classes/signalextractors/cfd.cpp \
    ./classes/signalextractors/tdc_extractor.cpp \
    ./classes/detector_analyzer/detector_analyzer_backend.cpp \
    ./classes/detector_analyzer/delayline_detector_analyzer_simple.cpp \
    ./classes/detector_analyzer/achimsorter_hex.cpp\
    ./classes/detector_analyzer/delayline_non_sorting.cpp\
    ./classes/momenta_calculators/momenta_calculator.cpp \
    ./classes/momenta_calculators/spectrometer.cpp

HEADERS += ./acqiris_converter.h \
    ./cass_acqiris.hpp \
    ./acqiristdc_converter.h \
    ./agat_deserializer.h \
    ./agattypes.hpp \
    ./lma_reader.h \
    ./lma_parser.h \
    ../cass/conversion_backend.h \
    ./classes/particle.h \
    ./classes/detector_backend.h \
    ./classes/delayline_detector.h \
    ./classes/tof_detector.h \
    ./classes/signal_producer.h \
    ./classes/signalextractors/signal_extractor.h \
    ./classes/signalextractors/cfd.h \
    ./classes/signalextractors/com.h \
    ./classes/signalextractors/tdc_extractor.h \
    ./classes/signalextractors/helperfunctionsforstdc.h \
    ./classes/detector_analyzer/detector_analyzer_backend.h \
    ./classes/detector_analyzer/delayline_detector_analyzer_simple.h \
    ./classes/detector_analyzer/delayline_non_sorting.h \
    ./classes/detector_analyzer/achimsorter_hex.h \
    ./classes/detector_analyzer/poscalculator.hpp \
    ./classes/momenta_calculators/spectrometer.h \
    ./classes/momenta_calculators/momenta_calculator.h

#INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

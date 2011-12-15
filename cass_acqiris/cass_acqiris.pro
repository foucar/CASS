# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

CASS_ROOT = ..

include( $${CASS_ROOT}/cass_config.pri )

TARGET = cass_acqiris
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

DEFINES += CASS_ACQIRIS_LIBRARY

INCLUDEPATH += ../cass \
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
SOURCES += ./acqiris_converter.cpp \
    ./acqiris_device.cpp \
    ./acqiristdc_converter.cpp \
    ./acqiristdc_device.cpp \
    ./agat_deserializer.cpp \
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
    ./cass_acqiris.h \
    ./acqiris_device.h \
    ./acqiristdc_converter.h \
    ./acqiristdc_device.h \
    ./agat_deserializer.h \
    ./map.hpp \
    ../cass/analysis_backend.h \
    ../cass/conversion_backend.h \
    ../cass/device_backend.h \
    ../cass/serializer.h \
    ./classes/particle.h \
    ./classes/channel.h \
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

headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

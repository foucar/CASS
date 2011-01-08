# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010 Lutz Foucar
CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )
TARGET = cass_acqiris
TEMPLATE = lib
DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib
QT -= core \
    gui
DEFINES += CASS_ACQIRIS_LIBRARY
INCLUDEPATH += \
    ../cass \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    . \
    ../LCLS
DEPENDPATH += \
    ../cass \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    .
SOURCES += \
    acqiris_converter.cpp \
    acqiris_device.cpp \
    ./classes/signal_producer.cpp \
    ./classes/signalextractors/signal_extractor.cpp \
    ./classes/signalextractors/com.cpp \
    ./classes/signalextractors/cfd.cpp \
    ./classes/detector_analyzer/delayline_detector_analyzer_simple.cpp \
    ./classes/detector_backend.cpp \
    ./classes/tof_detector.cpp

HEADERS += \
    acqiris_converter.h \
    cass_acqiris.h \
    acqiris_device.h \
    ../cass/analysis_backend.h \
    ../cass/conversion_backend.h \
    ../cass/device_backend.h \
    ../cass/serializer.h \
    ./classes/channel.h \
    ./classes/peak.h \
    ./classes/detector_backend.h \
    ./classes/delayline_detector.h \
    ./classes/tof_detector.h \
    ./classes/signal_producer.h \
    ./classes/signalextractors/signal_extractor.h \
    ./classes/signalextractors/cfd.h \
    ./classes/signalextractors/com.h \
    ./classes/signalextractors/helperfunctionsforstdc.h \
    ./classes/detector_analyzer/detector_analyzer_backend.h \
    ./classes/detector_analyzer/delayline_detector_analyzer_backend.h \
    ./classes/detector_analyzer/delayline_detector_analyzer_simple.h \
    ./classes/detector_analyzer/tof_analyzer_simple.h
headers.files = $$HEADERS
INSTALLS += target

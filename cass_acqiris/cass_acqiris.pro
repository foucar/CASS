# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_acqiris

CASS_ROOT = ../

include($${CASS_ROOT}/cass_config.pri )

QT            -= core gui

DEFINES       += CASS_ACQIRIS_LIBRARY
INCLUDEPATH   += ../cass ./classes ./classes/waveformanalyzer ./classes/detector_analyzer . ../LCLS
DEPENDPATH    += ../cass ./classes ./classes/waveformanalyzer ./classes/detector_analyzer .

SOURCES += acqiris_analysis.cpp \
           acqiris_converter.cpp \
           acqiris_device.cpp \
           ./classes/waveformanalyzer/cfd.cpp \
           ./classes/waveformanalyzer/com.cpp \
           ./classes/detector_analyzer/delayline_detector_analyzer_simple.cpp

HEADERS += acqiris_analysis.h \
           acqiris_converter.h \
           cass_acqiris.h  \
           acqiris_device.h \
           ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/device_backend.h \
           ../cass/serializer.h \
           ./classes/channel.h \
           ./classes/peak.h \
           ./classes/detector_backend.h \
           ./classes/delayline_detector.h \
           ./classes/tof_detector.h \
           ./classes/waveform_signal.h \
           ./classes/waveformanalyzer/waveform_analyzer_backend.h \
           ./classes/waveformanalyzer/results_backend.h \
           ./classes/waveformanalyzer/cfd.h \
           ./classes/waveformanalyzer/com.h \
           ./classes/waveformanalyzer/helperfunctionsforstdc.h \
           ./classes/detector_analyzer/detector_analyzer_backend.h \
           ./classes/detector_analyzer/delayline_detector_analyzer_backend.h \
           ./classes/detector_analyzer/delayline_detector_analyzer_simple.h \
           ./classes/detector_analyzer/tof_analyzer_simple.h \

headers.files   = $$HEADERS
libs.files     = libcass_acqiris.a

INSTALLS      += headers libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

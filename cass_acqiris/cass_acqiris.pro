# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_acqiris
CONFIG        += release
CONFIG        += thread warn_on exceptions rtti sse2 stl
CONFIG        += static staticlib
QT            -= core gui

CODECFORTR     = UTF-8
DEFINES       += CASS_ACQIRIS_LIBRARY
DEPENDPATH    +=  ../cass  ./classes ./classes/waveformanalyzer ./classes/detector_analyzer .
INCLUDEPATH   += ../LCLS ../cass ./classes ./classes/waveformanalyzer ./classes/detector_analyzer .
MOC_DIR        = ./obj
OBJECTS_DIR    = ./obj
QMAKE_STRIP    =
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += cass
VERSION        = 0.1.0

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

header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/lib
header.files   = $$HEADERS
libs.files     = libcass_acqiris.a
INSTALLS      += header libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

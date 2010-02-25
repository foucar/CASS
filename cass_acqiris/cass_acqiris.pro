# Copyright (C) 2009 Jochen Küpper
#modified Copyright (C) 2009 N Coppola,lmf
CONFIG += static
QT -= core gui
TEMPLATE = lib
TARGET = cass_acqiris
DEFINES += CASS_ACQIRIS_LIBRARY

OBJECTS_DIR = ./obj


SOURCES += acqiris_analysis.cpp \
           acqiris_converter.cpp \
           ./classes/waveformanalyzer/cfd.cpp \
           ./classes/waveformanalyzer/com.cpp \
           ./classes/detector_analyzer/delayline_detector_analyzer_simple.cpp \

HEADERS += acqiris_analysis.h \
           acqiris_converter.h \
           cass_acqiris.h  \
           acqiris_device.h \
           detector_backend.h \
           detector_analyzer_backend.h \
           ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/device_backend.h \
           ./classes/channel.h \
           ./classes/peak.h \
           ./classes/delayline_detector.h \
           ./classes/waveformanalyzer/waveform_analyzer_backend.h \
           ./classes/waveformanalyzer/cfd.h \
           ./classes/waveformanalyzer/com.h \
           ./classes/waveformanalyzer/helperfunctionsforstdc.h \
           ./classes/detector_analyzer/detector_analyzer_backend.h \
           ./classes/detector_analyzer/delayline_detector_analyzer_backend.h \
           ./classes/detector_analyzer/delayline_detector_analyzer_simple.h \

INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \
               ./classes \
               ./classes/waveformanalyzer \
               ./classes/detektorhitsorter \
               ./

INSTALLBASE     = /usr/local/cass
header.path     = $$INSTALLBASE/include
libs.path       = $$INSTALLBASE/libs

header.files    = $$HEADERS
libs.files      = libcass_acqiris*
INSTALLS        += header libs

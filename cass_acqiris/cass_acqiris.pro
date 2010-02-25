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
           ./classes/event/channel/channel.cpp \
           ./classes/event/peak/peak.cpp \
           ./classes/event/detector/detector.cpp \
           ./classes/waveformanalyzer/cfd.cpp \
           ./classes/waveformanalyzer/com.cpp \
           ./classes/detektorhitsorter/detektorhitsorter.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_quad.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_simple.cpp \

HEADERS += acqiris_analysis.h \
           acqiris_converter.h \
           cass_acqiris.h  \
           remi_device.h \
           ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/device_backend.h \
           ./classes/event/channel/channel.h \
           ./classes/event/peak/peak.h \
           ./classes/event/detector/detector.h \
           ./classes/waveformanalyzer/waveform_analyzer.h \
           ./classes/waveformanalyzer/cfd.h \
           ./classes/waveformanalyzer/com.h \
           ./classes/waveformanalyzer/helperfunctionsforstdc.h \
           ./classes/detektorhitsorter/detektorhitsorter.h \
           ./classes/detektorhitsorter/detektorhitsorter_quad.h \
           ./classes/detektorhitsorter/detektorhitsorter_simple.h \

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

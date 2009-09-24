# Copyright (C) 2009 Jochen Küpper

CONFIG += static
QT -= core gui
TEMPLATE = lib
DEFINES += CASS_REMI_LIBRARY

TARGET = cass_remi

SOURCES += remi_analysis.cpp \
           remi_converter.cpp \
           ./classes/event/remi_event.cpp \
           ./classes/event/channel/channel.cpp \
           ./classes/event/peak/peak.cpp \
           ./classes/event/detector/detector.cpp \
           ./classes/signalanalyzer/signal_analyzer.cpp \
           ./classes/signalanalyzer/softtdc_cfd.cpp \
           ./classes/signalanalyzer/softtdc_com.cpp \
           ./classes/detektorhitsorter/detektorhitsorter.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_quad.cpp \
           ./classes/detektorhitsorter/detektorhitsorter_simple.cpp \

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           remi_analysis.h \
           remi_converter.h \
           cass_remi.h  \
           ./classes/event/remi_event.h \
           ./classes/event/channel/channel.h \
           ./classes/event/peak/peak.h \
           ./classes/event/detector/detector.h \
           ./classes/signalanalyzer/signal_analyzer.h \
           ./classes/signalanalyzer/softtdc.h \
           ./classes/signalanalyzer/softtdc_cfd.h \
           ./classes/signalanalyzer/softtdc_com.h \
           ./classes/signalanalyzer/helperfunctionsforstdc.h \
           ./classes/detektorhitsorter/detektorhitsorter.h \
           ./classes/detektorhitsorter/detektorhitsorter_quad.h \
           ./classes/detektorhitsorter/detektorhitsorter_simple.h \

INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \
               ./classes/signalanalyzer \
               ./classes/detektorhitsorter \
               ./classes/event \
               ./classes/event/channel \
               ./classes/event/peak \
               ./classes/event/detector \
               ./

INSTALLBASE     = /usr/local/cass
header.path     = $$INSTALLBASE/include
libs.path       = $$INSTALLBASE/libs

header.files    = $$HEADERS
libs.files      = libcass_remi.a
INSTALLS        += header libs

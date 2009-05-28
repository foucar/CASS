# Copyright (C) 2009 Jochen Küpper

CONFIG += release thread warn_on

QT -= core gui

VERSION = 0.0.1

QMAKE_CXXFLAGS = -O5 -march=native -ftree-vectorize -fopenmp

TARGET = cass_remi

TEMPLATE = lib

DEFINES += CASS_LIBRARY

SOURCES += AnalysisBackend.cpp

HEADERS += AnalysisBackend.h \
           cass.h


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass*.dylib
INSTALLS      += header libs

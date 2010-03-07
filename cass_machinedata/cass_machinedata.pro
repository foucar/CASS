# Copyright (C) 2009 Jochen Küpper

CONFIG += static
QT -= core gui
TEMPLATE = lib
TARGET = cass_machinedata
DEFINES += CASS_MACHINEDATA_LIBRARY
VERSION = 0.0.1

OBJECTS_DIR = ./obj

SOURCES += machine_analysis.cpp \
           machine_converter.cpp \


HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           ../cass/serializer.h \
           machine_analysis.h \
           machine_converter.h \
           cass_machine.h \
           machine_device.h \


INCLUDEPATH += $$(LCLSSYSINCLUDE) \
               ../cass \



INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_machinedata.a
INSTALLS      += header libs

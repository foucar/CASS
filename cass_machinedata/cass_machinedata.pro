# Copyright (C) 2009 Jochen Küpper
# Copyright (C) 2009, 2010 Lutz Foucar

TEMPLATE       = lib
TARGET         = cass_machinedata

CASS_ROOT = ../

include($${CASS_ROOT}/cass_config.pri )

QT            -= core gui

DEFINES       += CASS_MACHINEDATA_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass


SOURCES += machine_analysis.cpp \
           machine_converter.cpp \


HEADERS += ../cass/analysis_backend.h \
           ../cass/conversion_backend.h \
           ../cass/serializer.h \
           machine_analysis.h \
           machine_converter.h \
           cass_machine.h \
           machine_device.h \


headers.files   = $$HEADERS
libs.files     = libcass_machinedata.a
INSTALLS      += headers libs




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

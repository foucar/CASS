# Copyright (C) 2009 Jochen Küpper
# Copyright (C) 2009, 2010 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET         = cass_machinedata
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $$INSTALLBASE/lib

QT            -= core gui

DEFINES       += CASS_MACHINEDATA_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass


SOURCES       += machine_converter.cpp \


HEADERS       += ../cass/analysis_backend.h \
                 ../cass/conversion_backend.h \
                 ../cass/serializer.h \
                 machine_converter.h \
                 cass_machine.h \
                 machine_device.h \


headers.files  = $$HEADERS
#INSTALLS      += headers target
INSTALLS      += target




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

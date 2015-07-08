# Copyright (C) 2009 Jochen Küpper
# Copyright (C) 2009, 2010, 2013 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET         = cass_machinedata
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $${PREFIX}/lib

QT            -= gui

DEFINES       += CASS_MACHINEDATA_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass


SOURCES       += machine_converter.cpp


HEADERS       += ../cass/conversion_backend.h \
                 ../cass/serializer.h \
                 machine_converter.h \
                 cass_machine.h \
                 machine_device.h


#INSTALLS      += target

QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

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
INCLUDEPATH   += ../cass ../LCLS ../cass/event ../cass/input
DEPENDPATH    += ../cass


SOURCES       +=


HEADERS       += \
                 cass_machine.hpp \


#INSTALLS      += target

QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

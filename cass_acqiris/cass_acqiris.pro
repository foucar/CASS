# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

CASS_ROOT = ..

include( $${CASS_ROOT}/cass_config.pri )

TARGET = cass_acqiris
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $${PREFIX}/lib

QT -= gui

DEFINES += CASS_ACQIRIS_LIBRARY

INCLUDEPATH += \
    ../cass \
    ../cass/event \
    ../cass/input \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    ./classes/momenta_calculators \
    . \
    ../LCLS

DEPENDPATH += ../cass \
    ./classes \
    ./classes/signalextractors \
    ./classes/detector_analyzer \
    .

HEADERS +=    ./cass_acqiris.hpp

#INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

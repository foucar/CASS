# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = epics
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $${PREFIX}/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += ./src/*.cc

HEADERS += ./*.hh

headers.files = $$HEADERS

#INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = cspaddata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += \
    ../../

SOURCES += \
    ./src/ElementV1.cc \
    ./src/ElementV2.cc \
    ./src/ElementHeader.cc \
    ./src/ConfigV2.cc \
    ./src/ConfigV3.cc \
    ./src/ConfigV4.cc \
    ./src/ElementIterator.cc \
    ./src/MiniElementV1.cc \
    ./src/CspadCompressor.cc

HEADERS += \
    ./ElementV1.hh \
    ./ElementV2.hh \
    ./ElementHeader.hh \
    ./ConfigV2.hh \
    ./ConfigV3.hh \
    ./ConfigV4.hh \
    ./ElementIterator.hh \
    ./MiniElementV1.hh \
    ./CspadCompressor.hh

headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

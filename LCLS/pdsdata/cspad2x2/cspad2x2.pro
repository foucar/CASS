# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = cspad2x2data
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += \
    ../../

SOURCES += \
    ./src/ConfigV1.cc \
    ./src/ConfigV2.cc \
    ./src/ElementHeader.cc \
    ./src/ElementV1.cc

HEADERS += \
    ./ConfigV1.hh \
    ./ConfigV1QuadReg.hh \
    ./ConfigV2.hh \
    ./ConfigV2QuadReg.hh \
    ./Detector.hh \
    ./ElementHeader.hh \
    ./ElementV1.hh \
    ./ProtectionSystem.hh

headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

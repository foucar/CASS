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
    ./src/ConfigV2.cc \
    ./src/ConfigV3.cc \
    ./src/ConfigV4.cc \
    ./src/ConfigV5.cc \
    ./src/CspadCompressor.cc \
    ./src/ElementHeader.cc \
    ./src/ElementIterator.cc \
    ./src/ElementV1.cc \
    ./src/ElementV2.cc \
    ./src/MiniElementV1.cc

HEADERS += \
    ./CompressorOMP.hh \
    ./ConfigV1.hh \
    ./ConfigV1QuadReg.hh \
    ./ConfigV2.hh \
    ./ConfigV2QuadReg.hh \
    ./ConfigV3.hh \
    ./ConfigV3QuadReg.hh \
    ./ConfigV4.hh \
    ./ConfigV5.hh \
    ./CspadCompressor.hh \
    ./Detector.hh \
    ./ElementHeader.hh \
    ./ElementIterator.hh \
    ./ElementV1.hh \
    ./ElementV2.hh \
    ./MiniElementV1.hh

headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

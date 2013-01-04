# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = pnccddata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += \
    ./src/ConfigV1.cc \
    ./src/ConfigV2.cc \
    ./src/FrameV1.cc

HEADERS += \
    ./ConfigV1.hh \
    ./ConfigV2.hh \
    ./FrameV1.hh


headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

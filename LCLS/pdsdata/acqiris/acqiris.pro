# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = acqdata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += \
    ./src/ConfigV1.cc \
    ./src/DataDescV1.cc \
    ./src/TdcConfigV1.cc \
    ./src/TdcDataV1.cc

HEADERS += \
    ./ConfigV1.hh \
    ./DataDescV1.hh \
    ./TdcConfigV1.hh \
    ./TdcDataV1.hh


headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

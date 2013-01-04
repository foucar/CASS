# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = evrdata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += \
    ./src/PulseConfig.cc \
    ./src/OutputMap.cc \
    ./src/ConfigV1.cc \ 
    ./src/ConfigV2.cc \ 
    ./src/ConfigV3.cc \
    ./src/DataV3.cc \
    ./src/EventCodeV3.cc \
    ./src/PulseConfigV3.cc

HEADERS += \
    ./PulseConfig.hh \
    ./OutputMap.hh \
    ./ConfigV1.hh \ 
    ./ConfigV2.hh \ 
    ./ConfigV3.hh \
    ./DataV3.hh \
    ./EventCodeV3.hh \
    ./PulseConfigV3.hh


headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

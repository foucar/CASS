# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = appdata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += \
    ./XtcMonitorServer.cc \
    ./XtcMonitorClient.cc \
    ./XtcMonitorMsg.cc
    
HEADERS += \
    ./XtcMonitorServer.hh \
    ./XtcMonitorClient.hh \
    ./XtcMonitorMsg.hh

headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../../../

include( $${CASS_ROOT}/cass_config.pri )

TARGET = xtcdata
TEMPLATE = lib

DESTDIR = $${CASS_ROOT}/lib
target.path = $$INSTALLBASE/lib

QT -= core \
    gui

INCLUDEPATH += ../../

SOURCES += \
    ./src/TypeId.cc \
    ./src/BldInfo.cc \
    ./src/XtcIterator.cc \
    ./src/Src.cc \
    ./src/TransitionId.cc \
    ./src/ClockTime.cc \
    ./src/TimeStamp.cc \
    ./src/Sequence.cc \
    ./src/ClockTime.cc \
    ./src/Level.cc \
    ./src/ProcInfo.cc \
    ./src/DetInfo.cc

HEADERS += \
    ./TypeId.hh \
    ./BldInfo.hh \
    ./XtcIterator.hh \
    ./Src.hh \
    ./TransitionId.hh \
    ./ClockTime.hh \
    ./TimeStamp.hh \
    ./Sequence.hh \
    ./ClockTime.hh \
    ./Level.hh \
    ./ProcInfo.hh \
    ./DetInfo.hh


headers.files = $$HEADERS

INSTALLS += target

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

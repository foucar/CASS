# Copyright (C) 2014 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET               = hdf5cbf-converter
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $${PREFIX}/bin
CONFIG              -= gui

QMAKE_CLEAN         += hdf5cbf-converter


SOURCES       += \
                 main.cpp \

HEADERS       += \
                 ../cass/cbf_handle.hpp \
                 ../cass/hdf5_handle.hpp \
                 ../cass/cl_parser.hpp

INCLUDEPATH   += \
                 $$PWD/../cass \


# Extra stuff if compiling with hdf5 support
hdf5 {
    LIBS      += -lhdf5
    DEFINES   += HDF5
}


INSTALLS      += target

QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET


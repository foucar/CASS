# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010,2013,2014,2015 Lutz Foucar


CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET              = jocassview
TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $${PREFIX}/bin

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG         += qtestlib
} else {
    QT             += widgets
    QT             += printsupport
    QT             += concurrent
    QT             += testlib
}

QMAKE_CLEAN        += jocassview

# generate the soap files
SOAP_INPUTFILE   = $$PWD/../cass/soapserver.h
SOAP_OUTPUTFILE  = soapCASSsoapProxy.cpp
SOAP_BIN         = $$GSOAP_BIN -C
include( $$PWD/../cass/soapfile_generator.pri )


SOURCES       += \
                 main.cpp \
                 led.cpp \
                 status_led.cpp \
                 data.cpp \
                 data_viewer.cpp \
                 zero_d_viewer_data.cpp \
                 zero_d_viewer.cpp \
                 minmax_control.cpp \
                 curve_plot.cpp \
                 one_d_viewer.cpp \
                 one_d_viewer_data.cpp \
                 track_zoomer_2d.cpp \
                 qwt_scroll_zoomer.cpp \
                 qwt_scroll_bar.cpp \
                 logcolor_map.cpp \
                 two_d_viewer.cpp \
                 two_d_viewer_data.cpp \
                 file_handler.cpp \
                 id_list.cpp \
                 data_source.cpp \
                 data_source_manager.cpp \
                 tcpclient.cpp \
                 jocassviewer.cpp \
                 ../cass/geom_parser.cpp \

HEADERS       += \
                 jocassviewer.h \
                 led.h \
                 status_led.h \
                 data.h \
                 data_viewer.h \
                 zero_d_viewer_data.h \
                 zero_d_viewer.h \
                 curve_plot.h \
                 one_d_viewer.h \
                 one_d_viewer_data.h \
                 logcolor_map.h \
                 two_d_viewer.h \
                 two_d_viewer_data.h \
                 minmax_control.h \
                 track_zoomer_2d.h \
                 qwt_scroll_zoomer.h \
                 qwt_scroll_bar.h \
                 file_handler.h \
                 id_list.h \
                 data_source.h \
                 data_source_manager.h \
                 tcpclient.h \
                 ../cass/cbf_handle.hpp \
                 ../cass/geom_parser.h \

INCLUDEPATH   += $$PWD/.. \
                 $$PWD/../cass \
                 $$PWD/../LCLS

LIBS          += -lgsoap++ -lgsoap \
                 -lqwt

# Extra stuff if compiling with hdf5 support
hdf5 {
    LIBS      += -lhdf5
    DEFINES   += HDF5
}

INSTALLS      += target

RESOURCES     += $$PWD/../jocassview/jocassview.qrc

# execute script that shows the current version derived from git
#version.target      = cass_version.h
version.commands    = $$PWD/update-version.sh
!offline:!online {
    version.commands = $$PWD/update-version.sh && ../cass/update-version.sh ../cass/cass_version.h
}
QMAKE_EXTRA_TARGETS+= version
PRE_TARGETDEPS     += version

QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

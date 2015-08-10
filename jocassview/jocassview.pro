# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010,2013,2014,2015 Lutz Foucar


CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET               = jocassview
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $${PREFIX}/bin

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG          += qtestlib
} else {
    QT              += widgets
    QT              += printsupport
    QT              += concurrent
    QT              += testlib
}

QMAKE_CLEAN         += jocassview

# generate the soap files
SOAP_INPUTFILE       = $${CASS_ROOT}/cass/soapserver.h
SOAP_OUTPUTFILE      = soapCASSsoapProxy.cpp
SOAP_BIN             = $$GSOAP_BIN -C
include( $${CASS_ROOT}/soapfile_generator.pri )


SOURCES             += main.cpp
SOURCES             += jocassviewer.cpp
HEADERS             += jocassviewer.h
SOURCES             += led.cpp
HEADERS             += led.h
SOURCES             += status_led.cpp
HEADERS             += status_led.h
SOURCES             += data.cpp
HEADERS             += data.h
SOURCES             += data_viewer.cpp
HEADERS             += data_viewer.h
SOURCES             += zero_d_viewer_data.cpp
HEADERS             += zero_d_viewer_data.h
SOURCES             += zero_d_viewer.cpp
HEADERS             += zero_d_viewer.h
SOURCES             += minmax_control.cpp
HEADERS             += minmax_control.h
SOURCES             += curve_plot.cpp
HEADERS             += curve_plot.h
SOURCES             += one_d_viewer.cpp
HEADERS             += one_d_viewer.h
SOURCES             += one_d_viewer_data.cpp
HEADERS             += one_d_viewer_data.h
SOURCES             += track_zoomer_2d.cpp
HEADERS             += track_zoomer_2d.h
SOURCES             += qwt_scroll_zoomer.cpp
HEADERS             += qwt_scroll_zoomer.h
SOURCES             += qwt_scroll_bar.cpp
HEADERS             += qwt_scroll_bar.h
SOURCES             += logcolor_map.cpp
HEADERS             += logcolor_map.h
SOURCES             += two_d_viewer.cpp
HEADERS             += two_d_viewer.h
SOURCES             += two_d_viewer_data.cpp
HEADERS             += two_d_viewer_data.h
SOURCES             += file_handler.cpp
HEADERS             += file_handler.h
SOURCES             += id_list.cpp
HEADERS             += id_list.h
SOURCES             += data_source.cpp
HEADERS             += data_source.h
SOURCES             += data_source_manager.cpp
HEADERS             += data_source_manager.h
SOURCES             += tcpclient.cpp
HEADERS             += tcpclient.h
SOURCES             += $${CASS_ROOT}/cass/geom_parser.cpp
HEADERS             += $${CASS_ROOT}/cass/geom_parser.h
HEADERS             += $${CASS_ROOT}/cass/cbf_handle.hpp
HEADERS             += $${CASS_ROOT}/cass/serializable.hpp
HEADERS             += $${CASS_ROOT}/cass/result.hpp

INCLUDEPATH         +=  $${CASS_ROOT}/cass

LIBS                += -lgsoap++ -lgsoap
LIBS                += -lqwt
LIBS                += -lz

# Extra stuff if compiling with hdf5 support
hdf5 {
    LIBS            += -lhdf5
    DEFINES         += HDF5
}

INSTALLS            += target

RESOURCES           += jocassview.qrc

# execute script that shows the current version derived from git
version.commands     = $$PWD/update-version.sh
!offline:!online {
    version.commands = $$PWD/update-version.sh && ../cass/update-version.sh ../cass/cass_version.h
}
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS      += version

QMAKE_CLEAN         += $$OBJECTS_DIR/*.o
QMAKE_CLEAN         += $$MOC_DIR/moc_*
QMAKE_CLEAN         += $$TARGET

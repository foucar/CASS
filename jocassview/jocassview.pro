# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010,2013 Lutz Foucar


CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET              = jocassview
TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $${PREFIX}/bin

CONFIG             += qtestlib

QMAKE_CLEAN        += jocassview

SOAPFiles.target    = soapCASSsoapProxy.cpp
SOAPFiles.commands  = @soapcpp2 -C -i $$PWD/../cass/soapserver.h
SOAPFiles.files    += soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
                      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
                      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
                      CASSsoap.clearHistogram.req.xml CASSsoap.clearHistogram.res.xml \
                      CASSsoap.getPostprocessorIds.req.xml CASSsoap.getPostprocessorIds.res.xml \
                      CASSsoap.writeini.req.xml CASSsoap.writeini.res.xml \
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl CASSsoap.receiveCommand.req.xml \
                      CASSsoap.controlDarkcal.req.xml
SOAPFiles.depends   = $$PWD/../cass/soapserver.h

SOAPFiles2.target   = soapC.cpp
SOAPFiles2.depends  = soapCASSsoapProxy.cpp

QMAKE_EXTRA_TARGETS+= SOAPFiles SOAPFiles2


SOURCES       += \
                 soapCASSsoapProxy.cpp \
                 soapC.cpp \
                 main.cpp \
                 led.cpp \
                 status_led.cpp \
                 main_window.cpp \
                 zero_d_viewer.cpp \
                 minmax_control.cpp \
                 one_d_viewer.cpp \
                 one_d_viewer_data.cpp \
                 track_zoomer_2d.cpp \
                 qwt_scroll_zoomer.cpp \
                 qwt_scroll_bar.cpp \
                 two_d_viewer.cpp \
                 two_d_viewer_data.cpp \
                 file_handler.cpp \
                 tcpclient.cpp \
                 jocassviewer.cpp \
#                 qwt_logcolor_map.cpp \

HEADERS       += \
                 jocassviewer.h \
                 main_window.h \
                 led.h \
                 status_led.h \
                 data_viewer.h \
                 zero_d_viewer.h \
                 one_d_viewer.h \
                 one_d_viewer_data.h \
                 two_d_viewer.h \
                 two_d_viewer_data.h \
                 minmax_control.h \
                 track_zoomer_2d.h \
                 qwt_scroll_zoomer.h \
                 qwt_scroll_bar.h \
                 file_handler.h \
                 soapH.h \
                 soapCASSsoapProxy.h \
                 soapStub.h \
                 tcpclient.h \

INCLUDEPATH   += $$PWD/.. \
                 $$PWD/../cass

LIBS          += -lgsoap++ -lgsoap \
                 -lqwt

# Extra stuff if compiling with hdf5 support
hdf5 {
    LIBS      += -lhdf5
    DEFINES   += HDF5
}

INSTALLS      += target

RESOURCES     += $$PWD/../jocassview/jocassview.qrc


QMAKE_CLEAN   += $$SOAPFiles.files
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010 Lutz Foucar


CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET              = jocassview
TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $${PREFIX}/bin


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
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
SOAPFiles.depends   = $$PWD/../cass/soapserver.h

SOAPFiles2.target   = soapC.cpp
SOAPFiles2.depends  = soapCASSsoapProxy.cpp

#versiontarget.target = $$PWD/../jocassview/update-version.sh
#versiontarget.commands = $$PWD/../jocassview/update-version.sh
#versiontarget.depends = FORCE

#PRE_TARGETDEPS     += $$PWD/../jocassview/update-version.sh
#QMAKE_EXTRA_TARGETS+= versiontarget
QMAKE_EXTRA_TARGETS+= SOAPFiles SOAPFiles2



SOURCES       += soapCASSsoapProxy.cpp \
                 soapC.cpp \
                 jocassview.cpp \
                 imageviewer.cpp \
                 plotwidget.cpp \
                 qwt_logcolor_map.cpp \
                 qwt_scroll_zoomer.cpp \
                 qwt_scroll_bar.cpp \
                 ../cass/postprocessing/id_list.cpp

HEADERS       += soapH.h \
                 imageviewer.h \
                 plotwidget.h \
                 qwt_scroll_zoomer.h \
                 qwt_scroll_bar.h \
                 soapCASSsoapProxy.h \
                 soapStub.h \
                 ../cass/postprocessing/id_list.h

FORMS         += imageviewer.ui

INCLUDEPATH   += $$PWD/.. \
                 $$PWD/../cass

LIBS          += -lgsoap++ -lgsoap \
                 -lqwt

# Extra stuff if compiling pp1000,pp1001
hdf5 {
    LIBS           += -lhdf5
    DEFINES        += HDF5
}

# TODO: THIS IS NOT CROSS-PLATFORM!!
#bin_copy.extra+= bash backup_copy.sh $${INSTALLBASE} $${TARGET}
#bin_copy.path  = ./
#INSTALLS      += target bin_copy
INSTALLS      += target

RESOURCES     += $$PWD/../jocassview/jocassview.qrc


QMAKE_CLEAN   += $$SOAPFiles.files
QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010 Lutz Foucar


CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET              = jocassview
TEMPLATE            = app
DESTDIR             = $${CASS_ROOT}/bin
target.path         = $$INSTALLBASE/bin


QMAKE_CLEAN        += jocassview
VERSION             = 0.0.1

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

QMAKE_CLEAN        += $$SOAPFiles.files

versiontarget.target = $$PWD/../jocassview/update-version.sh
versiontarget.commands = $$PWD/../jocassview/update-version.sh
versiontarget.depends = FORCE

PRE_TARGETDEPS     += $$PWD/../jocassview/update-version.sh
QMAKE_EXTRA_TARGETS+= versiontarget
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
                 $$PWD/../cass \
                 $$PWD/../cass_acqiris \
                 $$QWTINCDIR \
                 $$PWD/../LCLS

LIBS          += -lgsoap++ -lgsoap \
                 -L$${CASS_ROOT}/lib -lcass_acqiris \
                 -lcass_pnccd -lcass_ccd \
                 -lcass_machinedata \
                 -L$$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt \
                 -lappdata -lacqdata -lcamdata -levrdata -lpnccddata -lpulnixdata -lxtcdata \
                 -lgsoap++ -lgsoap \
                 -lqwt

# TODO: THIS IS NOT CROSS-PLATFORM!!
bin_copy.extra+= bash backup_copy.sh $${INSTALLBASE}
bin_copy.path  = ./
INSTALLS      += target bin_copy

RESOURCES     += $$PWD/../jocassview/jocassview.qrc



## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

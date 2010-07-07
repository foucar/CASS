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

SOAPFiles.target    = soapCASSsoapProxy
SOAPFiles.commands  = [ -r soapCASSsoapProxy.h ] && [ `find $$PWD/../cass/soapserver.h -newer soapCASSsoapProxy.h | grep -c soapserver` > "0" ] || soapcpp2 -C -i $$PWD/../cass/soapserver.h
SOAPFiles.files    += soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
                      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
                      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
                      CASSsoap.clearHistogram.req.xml CASSsoap.clearHistogram.res.xml \
                      CASSsoap.getPostprocessorIds.req.xml CASSsoap.getPostprocessorIds.res.xml \
                      CASSsoap.writeini.req.xml CASSsoap.writeini.res.xml \
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
QMAKE_CLEAN        += $$SOAPFiles.files

versiontarget.target = $$PWD/../jocassview/update-version.sh
versiontarget.commands = $$PWD/../jocassview//update-version.sh
versiontarget.depends = FORCE

PRE_TARGETDEPS     += $$PWD/../jocassview/update-version.sh
PRE_TARGETDEPS     += soapCASSsoapProxy
QMAKE_EXTRA_TARGETS+= versiontarget
QMAKE_EXTRA_TARGETS+= SOAPFiles



SOURCES       += jocassview.cpp \
                 imageviewer.cpp \
                 plotwidget.cpp \
                 qwt_logcolor_map.cpp \
                 qwt_scroll_zoomer.cpp \
                 qwt_scroll_bar.cpp \
                 soapC.cpp \
                 soapCASSsoapProxy.cpp \
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
                 $$QWTINCDIR

LIBS          += -lgsoap++ -lgsoap \
                 -L../cass_acqiris -lcass_acqiris \
                 -L../cass_pnccd -lcass_pnccd \
                 -L../cass_ccd -lcass_ccd \
                 -L../cass_machinedata -lcass_machinedata \
                 -L$$PWD/../LCLS/build/pdsdata/lib/x86_64-linux-static-opt \
                 -lappdata -lacqdata -lcamdata -levrdata -lpnccddata -lpulnixdata -lxtcdata \
                 -lgsoap++ -lgsoap \
                 -lqwt

bin.files      = $$TARGET
# TODO: THIS IS NOT CROSS-PLATFORM!!
bin.extra     +=bash backup_copy.sh $${INSTALLBASE}
INSTALLS      += bin

RESOURCES     += $$PWD/../jocassview/jocassview.qrc



## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

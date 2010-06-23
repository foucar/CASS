# Copyright (C) 2010 Jochen Küpper

TARGET              = jocassview
TEMPLATE            = app

CASS_ROOT = ../
include($${CASS_ROOT}/cass_config.pri )

QMAKE_CLEAN        += jocassview
VERSION             = 0.0.1

SOAPFiles.target    = soapCASSsoapProxy
SOAPFiles.commands  = find $$PWD/../cass/soapserver.h -newer soapCASSsoapProxy.h || soapcpp2 -C -i $$PWD/../cass/soapserver.h
SOAPFiles.files    += soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
                      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
                      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
QMAKE_CLEAN        += $$SOAPFiles.files

versiontarget.target = $$PWD/../jocassview/update-version.sh
versiontarget.commands = $$PWD/../jocassview//update-version.sh
versiontarget.depends = FORCE

PRE_TARGETDEPS     += $$PWD/../jocassview/update-version.sh soapCASSsoapProxy
QMAKE_EXTRA_TARGETS+= versiontarget SOAPFiles



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

bin.files      = jocassview
bin.path       = $$INSTALLBASE/bin
INSTALLS      += bin

RESOURCES     += $$PWD/../jocassview/jocassview.qrc

# TODO: THIS IS NOT CROSS-PLATFORM!!
QMAKE_POST_LINK = bash backup_copy.sh


## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

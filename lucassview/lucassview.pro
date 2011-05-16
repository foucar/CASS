# Copyright (C) 2011 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET               = lucassview
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $$INSTALLBASE/bin
CONFIG              -= gui


QMAKE_CLEAN        += lucassview
VERSION             = 0.0.1

#soap creation
SOAPFiles.target    = soapCASSsoapProxy.cpp
SOAPFiles.commands  = @soapcpp2 -C -i $$PWD/../cass/soapserver.h
SOAPFiles.files    += soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
                      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
                      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
                      CASSsoap.clearHistogram.req.xml CASSsoap.clearHistogram.res.xml \
                      CASSsoap.getPostprocessorIds.req.xml CASSsoap.getPostprocessorIds.res.xml \
                      CASSsoap.writeini.req.xml CASSsoap.writeini.res.xml \
                      ns.xsd CASSsoap.nsmap CASSsoap.wsdl \
                      CASSsoap.receiveCommand.req.xml CASSsoap.receiveCommand.res.xml

SOAPFiles.depends   = $$PWD/../cass/soapserver.h

SOAPFiles2.target   = soapC.cpp
SOAPFiles2.depends  = soapCASSsoapProxy.cpp

QMAKE_CLEAN        += $$SOAPFiles.files

QMAKE_EXTRA_TARGETS+= SOAPFiles SOAPFiles2

# dictionary creation
rootcint.target       = histo_updater_dict.cpp
rootcint.commands    += $(ROOTSYS)/bin/rootcint -f $$rootcint.target -c histo_updater.h histo_updater_linkdef.h
rootcint.depends      = histo_updater.h
rootcintecho.commands = @echo "Generating dictionary $$rootcint.target for histo_updater.h "
QMAKE_EXTRA_TARGETS  += rootcintecho rootcint
QMAKE_CLEAN          +=  histo_updater_dict.cpp histo_updater_dict.h



SOURCES       += soapCASSsoapProxy.cpp \
                 soapC.cpp \
                 histo_updater_dict.cpp \
                 main.cpp \
                 tcpclient.cpp \
                 histo_updater.cpp \
                 id_list.cpp \
                 histogram.cpp

HEADERS       += soapH.h \
                 histogram.h \
                 histo_updater.h \
                 id_list.h \
                 serializable.h \
                 serializer.h \
                 tcpclient.h

INCLUDEPATH   += $$(ROOTSYS)/include
LIBS          += -L$$(ROOTSYS)/lib -lCore -lCint -lRIO -lRint -lNet -lHist -lMathCore -lMatrix
LIBS          += -lgsoap++ -lgsoap


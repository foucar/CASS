# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010 Lutz Foucar

CODECFORTR          = UTF-8
CONFIG             += static
TARGET              = jk-client
TEMPLATE            = app
VERSION             = 0.0.1

OBJECTS_DIR         = ./obj
MOC_DIR             = ./obj
QMAKE_CLEAN        += $$OBJECTS_DIR/*.o $$MOC_DIR/moc_*

SOAPFiles.target    = soapStub.h
SOAPFiles.commands  = soapcpp2 -C -i ../cass/soapserver.h
SOAPFiles.depends   = FORCE
QMAKE_CLEAN         = soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
		      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
		      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
		      ns.xsd CASSsoap.nsmap CASSsoap.wsdl \
	              jk-client

PRE_TARGETDEPS     += soapStub.h
QMAKE_EXTRA_TARGETS+= SOAPFiles



SOURCES       += jk-client.cpp \
                 soapC.cpp \
                 soapCASSsoapProxy.cpp

HEADERS       += soapH.h \
                 soapCASSsoapProxy.h \
                 soapStub.h

LIBS          += -lgsoap++ -lgsoap

bin.path       = $$INSTALLBASE/bin
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs
bin.files      = cass.app
header.files   = $$HEADERS
libs.files     = libcass.a

INSTALLS      += bin




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

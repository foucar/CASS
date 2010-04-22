# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010 Lutz Foucar

TARGET              = jocassview
TEMPLATE            = app
CONFIG             += release
CONFIG             += thread warn_on exceptions rtti sse2 stl
CONFIG             += static
VERSION             = 0.0.1

CODECFORTR          = UTF-8
OBJECTS_DIR         = ./obj
MOC_DIR             = ./obj
QMAKE_CLEAN        += $$OBJECTS_DIR/*.o $$MOC_DIR/moc_* \
	              jocassview
QMAKE_STRIP         =

SOAPFiles.target    = soapCASSsoapProxy
SOAPFiles.commands  = newer soapCASSsoapProxy.h $$PWD/../cass/soapserver.h || soapcpp2 -C -i $$PWD/../cass/soapserver.h
SOAPFiles.files    += soapCASSsoapProxy.cpp soapCASSsoapProxy.h soapC.cpp soapH.h soapStub.h \
		      CASSsoap.getEvent.req.xml CASSsoap.getEvent.res.xml CASSsoap.getHistogram.req.xml \
		      CASSsoap.getHistogram.res.xml CASSsoap.getImage.req.xml CASSsoap.getImage.res.xml \
                      CASSsoap.quit.req.xml CASSsoap.quit.res.xml CASSsoap.readini.req.xml CASSsoap.readini.res.xml \
		      ns.xsd CASSsoap.nsmap CASSsoap.wsdl
QMAKE_CLEAN        += $$SOAPFiles.files

PRE_TARGETDEPS     += soapCASSsoapProxy
QMAKE_EXTRA_TARGETS+= SOAPFiles



SOURCES       += jocassview_cli.cpp \
                 soapC.cpp \
                 soapCASSsoapProxy.cpp

HEADERS       += soapH.h \
                 soapCASSsoapProxy.h \
                 soapStub.h

LIBS          += -lgsoap++ -lgsoap

bin.files      = jocassview
bin.path       = $$INSTALLBASE/bin
INSTALLS      += bin




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

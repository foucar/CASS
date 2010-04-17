# Copyright (C) 2010 Jochen Küpper
# Copyright (C) 2010 Lutz Foucar

TEMPLATE = app
CONFIG  += static
TARGET   = jk-client
DEFINES +=  DWITH_NONAMESPACES
VERSION  = 0.0.1
CODECFORTR = UTF-8

SOAPFiles.target = soapStub.h
SOAPFiles.commands = @echo "creating SOAP Client Files" && soapcpp2 -C -i ../cass/soapserver.h
SOAPFiles.depends = FORCE

PRE_TARGETDEPS += soapStub.h
QMAKE_EXTRA_TARGETS += SOAPFiles


OBJECTS_DIR = ./obj
MOC_DIR = ./obj

SOURCES       += jk-client.cpp \
                 soapC.cpp \
                 soapCASSsoapProxy.cpp

HEADERS       += soapH.h \
                 soapCASSsoapProxy.h \
                 soapStub.h

LIBS          += -lgsoap++ -lgsoap

INSTALLBASE    = /usr/local/cass
bin.path       = $$INSTALLBASE/bin
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs
bin.files      = cass.app
header.files   = $$HEADERS
libs.files     = libcass.a

INSTALLS      += bin



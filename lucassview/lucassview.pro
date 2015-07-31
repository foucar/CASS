# Copyright (C) 2011 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET               = lucassview
TEMPLATE             = app
DESTDIR              = $${CASS_ROOT}/bin
target.path          = $${PREFIX}/bin
CONFIG              -= gui

QMAKE_CLEAN         += lucassview

#soap creation
SOAPFiles.target     = soapCASSsoapProxy.cpp
SOAP_INPUTFILE       = $$PWD/../cass/soapserver.h
SOAP_OUTPUTFILE      = soapCASSsoapProxy.cpp
SOAP_BIN             = $$GSOAP_BIN -C
include( $$PWD/../cass/soapfile_generator.pri )

# dictionary creation
DICTIONARYFILES      = histo_updater.h
include( $$PWD/../cass/rootdict_generator.pri )


SOURCES       += \
                 main.cpp \
                 tcpclient.cpp \
                 histo_updater.cpp \
                 id_list.cpp \
                 histogram.cpp

HEADERS       += \
                 histogram.h \
                 histo_updater.h \
                 histo_updater_linkdef.h \
                 id_list.h \
                 serializable.h \
                 serializer.h \
                 tcpclient.h

LIBS          += $$system($$ROOTCONFIG_BIN --libs)
LIBS          += -lgsoap++ -lgsoap

INSTALLS      += target

QMAKE_CLEAN   += $$OBJECTS_DIR/*.o
QMAKE_CLEAN   += $$MOC_DIR/moc_*
QMAKE_CLEAN   += $$TARGET

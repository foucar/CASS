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
SOAP_INPUTFILE       = $${CASS_ROOT}/cass/soapserver.h
SOAP_OUTPUTFILE      = soapCASSsoapProxy.cpp
SOAP_BIN             = $$GSOAP_BIN -C
include( $${CASS_ROOT}/soapfile_generator.pri )

# dictionary creation
DICTIONARYFILES      = histo_updater.h
include( $${CASS_ROOT}/rootdict_generator.pri )


SOURCES             += main.cpp
SOURCES             += tcpclient.cpp
HEADERS             += tcpclient.h
SOURCES             += histo_updater.cpp
HEADERS             += histo_updater.h
HEADERS             += histo_updater_linkdef.h
SOURCES             += id_list.cpp
HEADERS             += id_list.h
HEADERS             += $${CASS_ROOT}/cass/result.hpp
HEADERS             += $${CASS_ROOT}/cass/serializable.hpp
HEADERS             += $${CASS_ROOT}/cass/serializer.hpp

INCLUDEPATH         +=  $${CASS_ROOT}/cass

LIBS                += $$system($$ROOTCONFIG_BIN --libs)
LIBS                += -lgsoap++ -lgsoap
LIBS                += -lz

INSTALLS            += target

QMAKE_CLEAN         += $$OBJECTS_DIR/*.o
QMAKE_CLEAN         += $$MOC_DIR/moc_*
QMAKE_CLEAN         += $$TARGET

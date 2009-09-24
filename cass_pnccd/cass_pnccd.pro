# Copyright (C) 2009 Jochen Küpper

CONFIG += static 
QT -= core gui
TEMPLATE = lib
TARGET = cass_pnccd
DEFINES += CASS_PNCCD_LIBRARY
VERSION = 0.0.1


SOURCES += pnccd_analysis.cpp \
           pnccd_converter.cpp

HEADERS += ../cass/analysis_backend.h \
           ../cass/parameter_backend.h \
           ../cass/conversion_backend.h \
           pnccd_analysis.h \
           pnccd_converter.h \
           cass_pnccd.h \
           ./classes/event/pnccd_event.h


INCLUDEPATH += ./ \
               ../cass \
               ./classes/event \
               $$(LCLSSYSINCLUDE)


INSTALLBASE    = /usr/local/cass
header.path    = $$INSTALLBASE/include
libs.path      = $$INSTALLBASE/libs

header.files   = $$HEADERS
libs.files     = libcass_pnccd.a
INSTALLS      += header libs

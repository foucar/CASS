# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 Nils Kimmel
# Copyright (C) 2009 Nicola Coppola
# Copyright (C) 2009, 2010 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

CONFIG(offline) {
    DEFINES        += OFFLINE
}

TARGET         = cass_pnccd
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $$INSTALLBASE/lib

QT            -= core

DEFINES       += CASS_PNCCD_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass

SOURCES       += pnccd_analysis.cpp \
                 pnccd_converter.cpp \

HEADERS       += ../cass/analysis_backend.h \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/serializer.h \
                 ../cass/pixel_detector.h \
                 pnccd_analysis.h \
                 pnccd_converter.h \
                 cass_pnccd.h \
                 pnccd_device.h \
                 pnccd_detector.h

headers.files  = $$HEADERS
target.path    = $$INSTALLBASE/lib
INSTALLS      += target
#INSTALLS      += headers target




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

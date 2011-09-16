# Copyright (C) 2011 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )


TARGET         = cass_pixeldetector
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $$INSTALLBASE/lib

QT            -= core

INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass

SOURCES       += \
                 coalesce_simple.cpp \
                 coalescing_base.cpp \
                 pixel_detector_new.cpp \
                 pixel_detector.cpp

HEADERS       += \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/serializer.h \
                 pixel_detector.h \
                 coalesce_simple.h \
                 coalescing_base.h \
                 pixel_detector_new.h \
                 pixel_detector.h

headers.files  = $$HEADERS
target.path    = $$INSTALLBASE/lib
INSTALLS      += target
#INSTALLS      += headers target




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

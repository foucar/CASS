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
                 pixeldetector_mask.cpp \
                 advanced_pixeldetector.cpp

HEADERS       += \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/serializer.h \
                 coalesce_simple.h \
                 coalescing_base.h \
                 pixeldetector_mask.h \
                 pixeldetector.hpp \
                 cass_pixeldetector.h \
                 advanced_pixeldetector.h

headers.files  = $$HEADERS
target.path    = $$INSTALLBASE/lib
INSTALLS      += target
#INSTALLS      += headers target




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

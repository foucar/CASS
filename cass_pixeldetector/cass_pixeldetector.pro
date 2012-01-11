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
                 common_data.cpp \
                 mapcreator_base.cpp \
                 mapcreators.cpp \
                 mapcreators_online.cpp \
                 frame_processor_base.cpp \
                 hll_frame_processor.cpp \
                 commonmode_calculator_base.cpp \
                 commonmode_calculators.cpp \
                 pixel_finder_base.cpp \
                 above_noise_finder.cpp \
                 pixel_finder_simple.cpp \
                 coalesce_simple.cpp \
                 coalescing_base.cpp \
                 pixeldetector_mask.cpp \
                 advanced_pixeldetector.cpp \
                 lcls_converter.cpp \
                 shm_deserializer.cpp \
                 frms6_reader.cpp \
                 raw_sss_reader.cpp

HEADERS       += \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/serializer.h \
                 ../cass/tcp_streamer.h \
                 ../cass/hlltypes.h \
                 common_data.h \
                 mapcreator_base.h \
                 mapcreators.h \
                 mapcreators_online.h \
                 frame_processor_base.h \
                 hll_frame_processor.h \
                 commonmode_calculator_base.h \
                 commonmode_calculators.h \
                 pixel_finder_base.h \
                 above_noise_finder.h \
                 pixel_finder_simple.h \
                 coalesce_simple.h \
                 coalescing_base.h \
                 pixeldetector_mask.h \
                 pixeldetector.hpp \
                 cass_pixeldetector.h \
                 advanced_pixeldetector.h \
                 lcls_converter.h \
                 shm_deserializer.h \
                 frms6_reader.h \
                 raw_sss_reader.h


headers.files  = $$HEADERS
target.path    = $$INSTALLBASE/lib
INSTALLS      += target
#INSTALLS      += headers target




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

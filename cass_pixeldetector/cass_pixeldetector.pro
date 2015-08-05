# Copyright (C) 2011, 2013 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )


TARGET         = cass_pixeldetector
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $${PREFIX}/lib

QT            -= gui

INCLUDEPATH   += ../cass ../LCLS ../cass/event ../cass/input
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
                 gaincalibration.cpp


HEADERS       += \
                 ../cass/cass_settings.h \
                 ../cass/serializer.hpp \
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
                 cass_pixeldetector.hpp \
                 advanced_pixeldetector.h \
                 gaincalibration.h

#INSTALLS      += target

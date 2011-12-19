# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009 N Coppola
# Copyright (C) 2009, 2010 Lutz Foucar

CASS_ROOT = ..
include( $${CASS_ROOT}/cass_config.pri )

TARGET         = cass_ccd
TEMPLATE       = lib
DESTDIR        = $${CASS_ROOT}/lib
target.path    = $$INSTALLBASE/lib

QT            -= core gui

DEFINES       += CASS_CCD_LIBRARY
INCLUDEPATH   += ../cass ../LCLS
DEPENDPATH    += ../cass

SOURCES       += ccd_analysis.cpp \
                 ccd_converter.cpp \
                 raw_sss_parser.cpp \
                 raw_sss_reader.cpp


HEADERS       += ../cass/analysis_backend.h \
                 ../cass/cass_settings.h \
                 ../cass/conversion_backend.h \
                 ../cass/device_backend.h \
                 ../cass/pixel_detector.h \
                 ../cass/serializer.h \
                 raw_sss_file_header.h \
                 raw_sss_reader.h \
                 raw_sss_parser.h \
                 ccd_analysis.h \
                 ccd_converter.h \
                 cass_ccd.h \
                 ccd_device.h

headers.files  = $$HEADERS
libs.files     = $$TARGET
INSTALLS      += target
#INSTALLS      += headers target

QMAKE_CLEAN        += $$SOAPFiles.files
QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET



## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

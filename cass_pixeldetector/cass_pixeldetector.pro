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



HEADERS       += cass_pixeldetector.hpp

#INSTALLS      += target

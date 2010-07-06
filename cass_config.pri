# Copyright (C) 2010 Stephan Kassemeyer

# this file points to configuration that will be read by all .pro files, 
# which contains all commonly used config parameters

exists ( cass_myconfig.pri ) {
    message ("Using cass_myconfig.pri")
    include(cass_myconfig.pri)
}
else {
    message ("Using cass_defaultconfig.pri")
    include(cass_defaultconfig.pri)
}

VERSION      = 0.1.0

CODECFORTR   = UTF-8

CONFIG(debug, debug|release) {
    DEFINES += DEBUG VERBOSE QT_DEBUG
    SUFFIX_STR = _d
}
else {
    DEFINES += NDEBUG QT_NO_DEBUG
}

MOC_DIR      = moc
OBJECTS_DIR  = obj$${SUFFIX_STR}

QMAKE_CLEAN += $$OBJECTS_DIR/*.o
QMAKE_CLEAN += $$MOC_DIR/moc_*
QMAKE_CLEAN += $$TARGET

bin.path     = $$INSTALLBASE/bin
libs.path    = $$INSTALLBASE/lib
headers.path = $$INSTALLBASE/include


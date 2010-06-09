# Copyright (C) 2010 Stephan Kassemeyer

# this file points to configuration that will be read by all .pro files, 
# which contains all commonly used config parameters

exists ( cass_myconfig.pri ) {
    include(cass_myconfig.pri)
}
else {
    include(cass_defaultconfig.pri)
}

VERSION      = 0.1.0

CODECFORTR   = UTF-8

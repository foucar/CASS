# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

include( cass_config.pri )

TEMPLATE          = subdirs
CONFIG           += ordered


offline|online {
    SUBDIRS       = LCLS
    SUBDIRS      += cass_acqiris
    SUBDIRS      += cass_pixeldetector
    SUBDIRS      += cass_machinedata
    SUBDIRS      += cass

    cass.depends  = LCLS
    cass.depends += cass_acqiris
    cass.depends += cass_pixeldetector
    cass.depends += cass_machinedata
}

DOCS{
    SUBDIRS      += doc
}

JoCASSView {
    SUBDIRS      += jocassview
}

LuCASSView {
    SUBDIRS      += lucassview
}


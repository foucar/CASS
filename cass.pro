# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

include( cass_config.pri )

TEMPLATE       = subdirs
CONFIG        += ordered
SUBDIRS        = LCLS \
                 cass_acqiris \
                 cass_pixeldetector \
                 cass_machinedata \
                 cass \
                 doc

cass.depends = LCLS \
               cass_acqiris \
               cass_pixeldetector \
               cass_machinedata

JoCASSView{
SUBDIRS       += jocassview
}

LuCASSView{
SUBDIRS       += lucassview
}


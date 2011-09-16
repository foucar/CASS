# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009, 2010, 2011 Lutz Foucar

include( cass_config.pri )

TEMPLATE       = subdirs
CONFIG        += ordered
SUBDIRS        = \
                 cass_acqiris \
                 cass_pixeldetector \
                 cass_ccd \
                 cass_pnccd \
                 cass_machinedata \
                 cass

JoCASSView{
SUBDIRS       += jocassview
}

LuCASSView{
SUBDIRS       += lucassview
}


## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

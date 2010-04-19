# Copyright (C) 2009, 2010 Jochen Küpper

CONFIG        += release
CONFIG        += thread warn_on exceptions rtti sse2 stl
CONFIG        += static staticlib
TEMPLATE       = subdirs
VERSION        = 0.1.0

QMAKE_STRIP    =

SUBDIRS        = cass_acqiris \
                 cass_ccd \
                 cass_pnccd \
                 cass_machinedata \
                 cass \
                 jk-client




## Local Variables:
## coding: utf-8
## mode: makefile
## fill-column: 100
## End:

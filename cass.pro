# Copyright (C) 2009, 2010Jochen Küpper

CONFIG += release
CONFIG += thread warn_on exceptions rtti sse2 stl

VERSION = 0.1.0

TEMPLATE = subdirs

INSTALLBASE    = /usr/local/cass

QMAKE_CFLAGS += -p
QMAKE_LFLAGS += -p

QMAKE_CXXFLAGS += -fopenmp -march=native
QMAKE_CXXFLAGS_DEBUG   += -g -O0
QMAKE_CXXFLAGS_RELEASE += -O3 -ftree-vectorize

SUBDIRS = cass_acqiris \
          cass_ccd \
          cass_pnccd \
          cass_machinedata \
          cass \
          jk-client

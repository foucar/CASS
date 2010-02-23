# Copyright (C) 2009 Jochen Küpper
# 10/9/2009 added cass_root

CONFIG += release
CONFIG += thread warn_on exceptions rtti sse2 stl
#CONFIG += debug

VERSION = 0.0.1

TEMPLATE = subdirs

INSTALLBASE    = /usr/local/cass

QMAKE_CFLAGS += -p
QMAKE_LFLAGS += -p

QMAKE_CXXFLAGS += -fopenmp -march=native
QMAKE_CXXFLAGS_DEBUG   += -g -O0
QMAKE_CXXFLAGS_RELEASE += -O3 -ftree-vectorize

SUBDIRS = \
          cass_remi \
          cass_ccd \
          cass_pnccd \
          cass_machinedata \
          cass \

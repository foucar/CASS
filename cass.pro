# Copyright (C) 2009 Jochen Küpper
# 10/9/2009 added cass_root

CONFIG += release
CONFIG += thread warn_on exceptions rtti sse2 stl

VERSION = 0.0.1

TEMPLATE = subdirs

INSTALLBASE    = /usr/local/cass

QMAKE_CXXFLAGS += -fopenmp -march=native
QMAKE_CXXFLAGS_DEBUG   += -g -O0
QMAKE_CXXFLAGS_RELEASE += -O3 -ftree-vectorize

#LFLAGS += -Wl,-rpath,/afs/desy.de/user/n/ncoppola/LCLS/new/release/build/pdsdata/lib/i386-linux

#HEADERS += TMyQButton.h
#CREATE_ROOT_DICT_FOR_CLASSES = $$HEADERS LinkDef.h
include("$(ROOTSYS)/include/rootcint.pri")

SUBDIRS = \
#          cass_root \
          cass_remi \
          cass_vmi \
          cass_pnccd \
#          cass_imageprocessor \
          cass_database \
          cass \
#          diode

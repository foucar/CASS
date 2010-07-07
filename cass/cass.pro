# Copyright (C) 2010 Lutz Foucar

TEMPLATE         = subdirs
CONFIG          += ordered
SUBDIRS          = cass_bin \
#                   cass_lib

cass_bin.subdir  = ./
cass_bin.file    = cass_bin.pro
#cass_bin.depends = cass_lib
#cass_lib.subdir  = ./
#cass_lib.file    = cass_lib.pro

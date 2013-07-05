# Copyright (C) 2010, 2013 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

TEMPLATE            = subdirs
CONFIG             += ordered

offline:SUBDIRS    += cass_offline_bin.pro
online:SUBDIRS     += cass_online_bin.pro

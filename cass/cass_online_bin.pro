# Copyright (C) 2010, 2013 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

message("Create online version")

TARGET           = cass_online
OBJECTS_DIR      = $${OBJECTS_DIR}_online

include( cass_bin.pri )

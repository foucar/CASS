# Copyright (C) 2013 Lutz Foucar

CASS_ROOT            = ..
include( $${CASS_ROOT}/cass_config.pri )

message("Create offline version")

TARGET           = cass_offline
DEFINES         += OFFLINE RINGBUFFER_BLOCKING
OBJECTS_DIR      = $${OBJECTS_DIR}_offline
CONFIG          *= is_offline

include( cass_bin.pri )

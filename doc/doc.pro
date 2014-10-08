# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../

include( $${CASS_ROOT}/cass_config.pri )

# Dummy as we are only generating the doc target
TEMPLATE = subdirs
 
## Install documentation
doc.path     = $$PREFIX/doc/

## the comamnd that will be executed when make install is called
doc.extra    = (cat Doxyfile && echo "PROJECT_NUMBER=`unset GIT_DIR; unset GIT_WORK_TREE; git describe --abbrev=4`") | $$DOXYGEN_BIN - && cp -r $$PWD/doxygen/html $$PREFIX/doc/.

## when one wants to create the docu with make doc
doc.commands = (cat Doxyfile && echo "PROJECT_NUMBER=`unset GIT_DIR; unset GIT_WORK_TREE; git describe --abbrev=4`") | $$DOXYGEN_BIN -

QMAKE_EXTRA_TARGETS += doc
INSTALLS            += doc

QMAKE_DISTCLEAN     += -r $$PWD/doxygen

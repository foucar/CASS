# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../

include( $${CASS_ROOT}/cass_config.pri )

# Dummy as we are only generating the doc target
TEMPLATE = subdirs
 
## Install documentation
doc.path     = $$PREFIX/doc/

## for now don't specify any files as we are taking care of the copying ourselves
#doc.files    = $$PWD/doxygen/html/*

## If you don't specify this, all files must exist when you run qmake or else they will
## just silently be ignored. When the files don't exist install -m blah will be
## used, which doesn't copy directories. Therefore the copy command is given
## explicitly in the extra session below. The extra will be used when make
## install is called.
#doc.CONFIG  += no_check_exist

## the comamnd that will be executed when make install is called
doc.extra    = (cat Doxyfile && echo "PROJECT_NUMBER=`unset GIT_DIR; unset GIT_WORK_TREE; git describe --abbrev=4`") | $$DOXYGEN - && cp -r $$PWD/doxygen/html $$PREFIX/doc/.

## when one wants to create the docu with make doc
doc.commands = (cat Doxyfile && echo "PROJECT_NUMBER=`unset GIT_DIR; unset GIT_WORK_TREE;  git describe --abbrev=4`") | $$DOXYGEN -

QMAKE_EXTRA_TARGETS += doc
INSTALLS            += doc

QMAKE_DISTCLEAN     += -r $$PWD/doxygen

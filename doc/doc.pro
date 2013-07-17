# Copyright (C) 2013 Lutz Foucar

CASS_ROOT = ../

include( $${CASS_ROOT}/cass_config.pri )

# Dummy, just to get custom compilers to work in qmake
TEMPLATE = lib
TARGET = Dummy
 
## Compiler for doxygen docu
#doc_builder.name = doxygen
##doc_builder.input = TEX
#doc_builder.input = Doxyfilej
##doc_builder.output = ${QMAKE_FILE_BASE}.pdf
#doc_builder.output = dummy
#doc_builder.commands = doxygen
# 
## This makes the custom compiler run before anything else
##doc_builder.CONFIG += target_predeps
# 
#doc_builder.variable_out = doc.files
#doc_builder.clean = ${CASS_ROOT}/doxgyen 
#QMAKE_EXTRA_COMPILERS += doc_builder
# 
## Install documentation
#doc.path = $$PREFIX/doc/
## If you don't specify this, all files must exist when you run qmake or else they will
## just silently be ignored
#doc.CONFIG += no_check_exist
# 
#INSTALLS += doc

dox.target = doc
dox.commands = (cat Doxyfile && echo "PROJECT_NUMBER=`env -i git describe`") | doxygen -
dox.depends =

QMAKE_EXTRA_TARGETS += dox

## Install documentation
docu.path = $$PREFIX/doc/
## If you don't specify this, all files must exist when you run qmake or else they will
## just silently be ignored
docu.files   = $$PWD/doxygen/*
docu.CONFIG += no_check_exist
 
INSTALLS += docu

QMAKE_CLEAN += $$PWD/doxygen/

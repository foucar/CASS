# This function builds the root dictornary files. Ensure that the ROOTCINT_BIN
# variable contains the the rootcint binary.
# The DICTIONARYFILES variable needs to contain the list of files for which to
# generate the root dictionaries

DictGenerator.input         = DICTIONARYFILES
DictGenerator.output        = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}_dict.cpp
DictGenerator.commands      = $$ROOTCINT_BIN -f ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}_dict.cpp -c ${QMAKE_FILE_IN} ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}_linkdef.h
DictGenerator.variable_out  = SOURCES
DictGenerator.clean        += ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}_dict.*
QMAKE_EXTRA_COMPILERS      += DictGenerator

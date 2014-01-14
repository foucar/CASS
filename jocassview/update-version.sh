#!/bin/bash

OUTFILE=jocassview_version.h
INCLUDEGUARD=_JOCASSVIEW_VERSION

unset GIT_DIR
unset GIT_WORK_TREE
VERSION=$(git describe --abbrev=4)

if [ -f ${OUTFILE} ] && [ $(cat ${OUTFILE} | grep ${VERSION} | wc -l) -eq 1 ]; then
  echo "${OUTFILE} does contain the latest information."
else
  echo "Generating file for VERSION: v${VERSION}"
  echo "#ifndef ${INCLUDEGUARD}
#define ${INCLUDEGUARD}

namespace jocassview
{
  std::string VERSION(\"v${VERSION}\");
}

#endif" > ${OUTFILE}
fi

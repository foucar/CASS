# adapted from http://stackoverflow.com/questions/2288292/qmake-project-dependencies-linked-libraries
# Copyright (C) 2013 Lutz Foucar

# This function sets up the dependencies for libraries that are built with
# this project.  Specify the libraries you need to depend on in the variable
# DEPENDENCY_LIBRARIES and this will add

SONAME=a

for(dep, DEPENDENCY_LIBRARIES) {
    message($$TARGET depends on $$dep ($${CASS_ROOT}/lib/lib$${dep}.$${SONAME}))
    LIBS += $${CASS_ROOT}/lib/lib$${dep}.$${SONAME}
    PRE_TARGETDEPS += $${CASS_ROOT}/lib/lib$${dep}.$${SONAME}
}

# Copyright (C) 2009, 2010 Jochen Küpper
# Copyright (C) 2009, 2010, 2011, 2015 Lutz Foucar

include( cass_config.pri )

TEMPLATE          = subdirs
CONFIG           += ordered



LCLSLibrary {
    SUBDIRS      += LCLS
}

offline|online {
    SUBDIRS      += cass

}

DOCS {
    SUBDIRS      += doc
}

JoCASSView {
    SUBDIRS      += jocassview
}

LuCASSView {
    SUBDIRS      += lucassview
}


// Copyright (C) 2009 lmf

#ifndef CASS_DEVICEBACKEND_H
#define CASS_DEVICEBACKEND_H

#include "cass.h"
#include <stdint.h>

namespace cass
{
    class CASSSHARED_EXPORT DeviceBackend
    {
    public:
        virtual ~DeviceBackend() {}
    };
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

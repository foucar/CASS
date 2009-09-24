// Copyright (C) 2009 Jochen Küpper,lmf

#ifndef CASS_ANALYSISBACKEND_H
#define CASS_ANALYSISBACKEND_H

#include "cass.h"

namespace cass
{
    class CASSEvent;

    class CASSSHARED_EXPORT AnalysisBackend
    {
    public:
        AnalysisBackend()           {}
        virtual ~AnalysisBackend()  {}

        virtual void init() = 0;

        virtual void operator()(CASSEvent*) = 0;
    };
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

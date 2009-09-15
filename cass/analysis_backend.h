// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_ANALYSISBACKEND_H
#define CASS_ANALYSISBACKEND_H

#include "cass.h"


namespace cass
{
    class CASSEvent;

    /** @class abstract base for all data analysis backend parameter sets

    @author Jochen Küpper
    @version 0.1
    */
    class CASSSHARED_EXPORT ParameterBackend
    {
    public:
        //virtual ParameterBackend()=0;
        virtual ~ParameterBackend() {}
    };


    /** @class abstract base for all data analysis backends

    @author Jochen Küpper
    @version 0.1
    */
    class CASSSHARED_EXPORT AnalysisBackend
    {
    public:
        virtual ~AnalysisBackend() {}

        /** initialize AnalysisBackend with new set of parameters */
        virtual void init(const ParameterBackend*) = 0;

        /* analyse dataset
        @param data cass Event
        */
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

// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_ANALYSISBACKEND_H
#define CASS_ANALYSISBACKEND_H

#include "cass.h"


namespace cass {

/** @class abstract base for all data analysis backend parameter sets

@author Jochen Küpper
@version 0.1
*/
class CASSSHARED_EXPORT BackendParameter {
};


/** @class abstract base for all data analysis backends

@author Jochen Küpper
@version 0.1
*/
class CASSSHARED_EXPORT AnalysisBackend {

    virtual ~AnalysisBackend() = 0;

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const BackendParameter& param) = 0;

    /* analyse dataset

    @param data Raw data to be analysed
    @return analysed data
    */
    virtual void * operator()(const void *data) = 0;
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

// Copyright (C) 2009 Jochen Küpper

#ifndef ANALYSISBACKEND_H
#define ANALYSISBACKEND_H

#include "cass.h"


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

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const BackendParameter& param) = 0;

    /* analyse dataset

     @param data Raw data to be analysed
     @return analysed data
     */
    virtual void * operator()(const void *data) = 0;
};


#endif // ANALYSISBACKEND_H

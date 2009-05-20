/*
 *  AnalysisBackend.h
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Jochen Küpper. All rights reserved.
 *
 */


/** @class abstract base for all data analysis backend parameter sets
 
 @author Jochen Küpper
 @version 0.1
 */
class BackendParameter {
};



/** @class abstract base for all data analysis backends
 
 @author Jochen Küpper
 @version 0.1
 */
class AnalysisBackend {

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const BackendParameter& param) = 0;
    
    /* analyse dataset
     
     @param data Raw data to be analysed
     @return analysed data
     */
    virtual void * operator()(const void *data) = 0;
};
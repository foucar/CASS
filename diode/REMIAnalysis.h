/*
 *  REMIAnalysis.h
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Jochen Küpper. All rights reserved.
 *
 */

#include <vector>
#include "AnalysisBackend.h"


/** @class REMI backend parameter sets
 
 @author Jochen Küpper
 @version 0.1
 */
class REMIParameter : BackendParameter {
public:
    std::pair<double, double> timerange;
};



/** alias for LCLS REMI data format container */
class RawREMIData {
public:
    char data[0xffff];
};



/** @class REMI analysis signal
 
 @author Jochen Küpper
 @version 0.1
 */
class REMISignal {
public:
    double x, y, t;
};





/** @class REMI analysis backend
 
 @author Jochen Küpper
 @version 0.1
 */
class REMIAnalysis : AnalysisBackend {
public:
    
    REMIAnalysis(const REMIParameter& param);
    
    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const REMIParameter& param);
    
    /* analyse dataset
     
     @param data Raw data to be analysed
     @return analysed data
     */
    virtual std::vector<REMISignal> operator()(const RawREMIData& data);
};
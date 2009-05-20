/*
 *  REMIAnalysis.h
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Jochen Küpper. All rights reserved.
 *
 */

#include <map>
#include <string>
#include <vector>
#include "AnalysisBackend.h"


/** @class REMI backend parameter sets
 
 @author Jochen Küpper
 @version 0.1
 */
class REMIParameter : BackendParameter {
public:
    /* a dictionary of all user settings
     
     The following entries (keys) must be present:
     - "Scalefactor U"
     - "Scalefactor V"
     - Scalefactor W
     ...
     
     The following entries (keys) are used if available:
     <none>
     */
    std::map<std::string, double> settings;
};



/** REMI data container */
class RawREMIData {
public:
    char data[0xfff]; // <-- this is a dummy -- the class would look like this:
    // Pds::Acqiris::ConfigV1& config;
    // Pds::Acqiris::DataDescV1& data;    
};



/** @class REMI analysis signal
 
 @author Jochen Küpper
 @version 0.1
 */
class REMISignal {
public:
    double x, y, t;
};



enum HistogramType {
    FWHM_U1, FWHM_U2, FWHM_U3
};



/* @class histogram container */
class Histogram {
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
    
    /* provide analysis histogram
     
     Return the specified histogram from the last processed event.
     
     @param type Which histogram do we want
     @return histogram data
     */
    Histogram histogram(HistogramType type);
};

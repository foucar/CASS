// Copyright (C) 2009 Jochen Küpper


#ifndef REMIANALYSIS_H
#define REMIANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_remi.h"
#include "AnalysisBackend.h"

namespace cass {
namespace REMI {

/** @class REMI backend parameter sets

@author Jochen Küpper
@version 0.1
*/
class Parameter : cass::BackendParameter {
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
class RawData {
public:
    char data[0xfff]; // <-- this is a dummy -- the class would look like this:
    // Pds::Acqiris::ConfigV1& config;
    // Pds::Acqiris::DataDescV1& data;
};



/** @class REMI analysis signal

@author Jochen Küpper
@version 0.1
*/
class Signal {
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
class CASS_REMISHARED_EXPORT Analysis : cass::AnalysisBackend
{
public:

    Analysis(const Parameter& param);

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const Parameter& param);

    /* analyse dataset

    @param data Raw data to be analysed
    @return analysed data
    */
    virtual std::vector<Signal> operator()(const RawData& data);

    /* provide analysis histogram

    Return the specified histogram from the last processed event.

    @param type Which histogram do we want
    @return histogram data
    */
    Histogram histogram(HistogramType type);
};


}
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

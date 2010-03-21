// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __HISTOGRAM_GETTER_H__
#define __HISTOGRAM_GETTER_H__

#include <string>

#include "cass.h"
#include "post_processor.h"

namespace cass
{
class Serializer;
class Histogram;


/** Histogram retrievel parameters

@author Jochen Küpper
*/
struct HistogramParameter {

    HistogramParameter(size_t _type)
        : type(_type)
        {};

    size_t type;
};




class CASSSHARED_EXPORT HistogramGetter
{
public:

    HistogramGetter(const cass::PostProcessors::histograms_t& histograms)
        : _histograms(histograms)
        {};

    const std::string operator()(const HistogramParameter&) const;

protected:

    const cass::PostProcessors::histograms_t &_histograms;
};

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

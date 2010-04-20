// Copyright (C) 2010 Jochen Küpper

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"
#include "postprocessing/postprocessor.h"


using namespace cass;


const std::string HistogramGetter::operator()(const HistogramParameter& hp) const
{
    Serializer serializer;
    // serialize the wanted histogram using the serializer
    PostProcessors::histograms_t::const_iterator iter(_histograms.find(hp.type));
    iter->second->serialize(serializer);
    //return the buffer (std::string) of the serializer
    return serializer.buffer();
}


QImage HistogramGetter::qimage(const HistogramParameter& hp) const
{
    // get an iterator to the requested histogram
    PostProcessors::histograms_t::const_iterator iter(_histograms.find(hp.type));
    // and return the QImage of that histogram
    return dynamic_cast<Histogram2DFloat *>(iter->second)->qimage();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

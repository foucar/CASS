#include "histogram_getter.h"
#include "serializer.h"
#include "histogram.h"


using namespace cass;


const std::string HistogramGetter::operator()(const HistogramParameter& hp) const
{
    //create a serializer that will serialize the cassevent//
    Serializer serializer;
    //serialize the wanted histogram using the serializer//
#warning: we need to decide how to retrieve a sub histogram a given postanalyzer
    std::map<std::pair<size_t, size_t>, HistogramBackend*>::const_iterator iter(_histograms.find(std::make_pair(hp.type,0)));

    iter->second->serialize(serializer);
    //return the buffer (std::string) of the serializer)
    return serializer.buffer();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

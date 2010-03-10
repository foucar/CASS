#include "histogram_getter.h"
#include "serializer.h"
#include "histogram.h"


cass::HistogramGetter::HistogramGetter(cass::PostProcessor::histograms_t& histos)
  :_histograms(histos)
{
}

const std::string cass::HistogramGetter::operator()(const TCP::HistogramParameter& hp)
{
  //create a serializer that will serialize the cassevent//
  Serializer serializer;
  //serialize the wanted histogram using the serializer//
  #warning: we need to decide how to retrieve a sub histogram a given postanalyzer
  _histograms[std::make_pair(hp.type,0)]->serialize(serializer);
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

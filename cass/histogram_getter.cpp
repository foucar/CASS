#include "histogram_getter.h"
#include "serializer.h"
#inlcude "histogram.h"


cass::HistogramGetter::HistogramGetter(std::map<size_t,Histogram*>& histos)
  :_histograms(histos)
{
}

const std::string cass::HistogramGetter::operator()(const TCP::HistogramParameter& hp)
{
  //create a serializer that will serialize the cassevent//
  Serializer serializer;
  //serialize the wanted histogram using the serializer//
  _histograms[hp.type]->serialize(serializer);
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

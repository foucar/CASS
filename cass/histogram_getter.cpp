// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include <stdexcept>

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"


using namespace cass;


const std::pair<size_t, std::string> HistogramGetter::operator()(const HistogramParameter& hp) const
{
  PostprocessorBackend &pp
      (_postprocessors->getPostProcessor(hp.key));
  const HistogramBackend & pp.getHist(hp.eventId);
  Serializer serializer;
  size_t dim(pp.dimension());
  pp.serialize(serializer);
  return make_pair(dim, serializer.buffer());
}

void HistogramGetter::clear(const HistogramParameter& hp) const
{
  // retrieve iterator pointing to postprocessor containing the histogram //
  PostProcessors::postprocessors_t::const_iterator iter
      (_postprocessors->getPostProcessor(hp.key));
  // clear the histograms of the postprocessor //
  if (iter->second)
    iter->second->clear();
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

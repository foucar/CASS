// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

/**
 * @file histogram_getter.cpp file contains definition retriever classes
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"
#include "backend.h"
#ifdef JPEG_CONVERSION
#include <sys/time.h>
#endif //JPEG_CONVERSION

using namespace cass;

const std::pair<size_t, std::string> HistogramGetter::operator()(const HistogramParameter& hp) const
{
  QReadLocker(&_postprocessors->lock);
  PostprocessorBackend &pp
      (_postprocessors->getPostProcessor(hp.key));
  const HistogramBackend &hist (pp.getHist(hp.eventId));
  Serializer serializer;
  size_t dim(hist.dimension());
  hist.serialize(serializer);
  return make_pair(dim, serializer.buffer());
}

void HistogramGetter::clear(const HistogramParameter& hp) const
{
  // retrieve iterator pointing to postprocessor containing the histogram //
  PostprocessorBackend &pp
      (_postprocessors->getPostProcessor(hp.key));
  // clear the histograms of the postprocessor //
  pp.clearHistograms();
}


#ifdef JPEG_CONVERSION
std::vector<JOCTET>* HistogramGetter::jpegImage(const HistogramParameter& hp)
{
  #define jpeg_cache_time 3 // seconds
  timeval thetime_struct;
  gettimeofday(&thetime_struct, 0);
  double thetime = thetime_struct.tv_sec + double(thetime_struct.tv_usec/1000000.0);
  bool cacheexists = (jpeg_cache.find(hp.key)!=jpeg_cache.end()); 
  if ( !cacheexists || (thetime-jpeg_cache_times[hp.key]>=jpeg_cache_time) ) {
    std::cout << "TTTTTTTTT: " << thetime << " " << jpeg_cache_times[hp.key] << " " << hp.key <<  thetime-jpeg_cache_times[hp.key] << std::endl;
    QReadLocker(&_postprocessors->lock);
    PostprocessorBackend &pp
        (_postprocessors->getPostProcessor(hp.key));
    const HistogramBackend &hist (pp.getHist(hp.eventId));
    if (cacheexists)
      delete jpeg_cache[hp.key];
    jpeg_cache[hp.key] = hist.jpegImage();
    jpeg_cache_times[hp.key] = thetime;
  }
  return jpeg_cache[hp.key];
}
#endif //JPEG_CONVERSION


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

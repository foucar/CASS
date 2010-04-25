// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"
#include "postprocessing/postprocessor.h"


using namespace cass;


const std::string HistogramGetter::operator()(const HistogramParameter& hp) const
{
    // check out histograms storage map
    PostProcessors *pp(PostProcessors::instance());
    const PostProcessors::histograms_t& hist(pp->histograms_checkout());
    pp->validate(hp.type);
    // serialize the wanted histogram using the serializer
    PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
    Serializer serializer;
    iter->second->serialize(serializer);
    // release and return
    pp->histograms_release();
    return serializer.buffer();
}


QImage HistogramGetter::qimage(const HistogramParameter& hp) const
{
    // check out histograms storage map
    PostProcessors *pp(PostProcessors::instance());
    const PostProcessors::histograms_t& hist(pp->histograms_checkout());
    // make sure the requested histogram is valid
    pp->validate(hp.type);
    // create the image
    PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
    // create the QImage, release, return
    QImage qi(dynamic_cast<Histogram2DFloat *>(iter->second)->qimage());
    pp->histograms_release();
    return qi;
}

void HistogramGetter::clear(const HistogramParameter& hp) const
{
  // check out histograms storage map//
  PostProcessors *pp(PostProcessors::instance());
  const PostProcessors::histograms_t& hist(pp->histograms_checkout());
  // make sure the requested histogram is valid//
  pp->validate(hp.type);
  // retrieve iterator pointing to histogram//
  PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
  // clear the histogram//
  iter->second->clear();
  // make sure that the histogram is accessable from others again//
  pp->histograms_release();
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

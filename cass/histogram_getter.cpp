// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include <stdexcept>

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"


using namespace cass;


const std::pair<size_t, std::string> HistogramGetter::operator()(const HistogramParameter& hp) const
{
    // check out histograms storage map
    const PostProcessors::histograms_t& hist(_postprocessors->histograms_checkout());
    _postprocessors->validate(hp.type);
    PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
    // get dimension
    size_t dim(iter->second->dimension());
    // serialize the wanted histogram using the serializer
    Serializer serializer;
    iter->second->serialize(serializer);
    // release and return
    _postprocessors->histograms_release();
    return make_pair(dim, serializer.buffer());
}


QImage HistogramGetter::qimage(const HistogramParameter& hp) const
{
    // check out histograms storage map
    const PostProcessors::histograms_t& hist(_postprocessors->histograms_checkout());
    // make sure the requested histogram is valid
    _postprocessors->validate(hp.type);
    // create the image
    PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
    //check wether requested histgogram is truly a 2d histogram//
    if (iter->second->dimension() != 2) {
      _postprocessors->histograms_release();
      throw std::invalid_argument(QString("requested histogram %1 is not a 2d histogram").arg(hp.type).toStdString());
    }
    // create the QImage, release, return
    QImage qi(reinterpret_cast<Histogram2DFloat *>(iter->second)->qimage());
    _postprocessors->histograms_release();
    return qi;
}


void HistogramGetter::clear(const HistogramParameter& hp) const
{
  // check out histograms storage map
  const PostProcessors::histograms_t& hist(_postprocessors->histograms_checkout());
  // make sure the requested histogram is valid//
  _postprocessors->validate(hp.type);
  // retrieve iterator pointing to histogram//
  PostProcessors::histograms_t::const_iterator iter(hist.find(hp.type));
  // clear the histogram//
  iter->second->clear();
  // make sure that the histogram is accessable from others again//
  _postprocessors->histograms_release();
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

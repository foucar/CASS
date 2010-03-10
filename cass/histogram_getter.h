//Copyright (C) 2010 lmf

#ifndef __HISTOGRAM_GETTER_H__
#define __HISTOGRAM_GETTER_H__

#include <string>

#include "cass.h"
#include "tcpserver.h"
#include "post_processor.h"

namespace cass
{
  class Serializer;
  class Histogram;

  class CASSSHARED_EXPORT HistogramGetter
  {
  public:
    HistogramGetter(cass::PostProcessor::histograms_t &histograms);

  public:
    const std::string operator()(const cass::TCP::HistogramParameter&);

  private:
    cass::PostProcessor::histograms_t &_histograms;
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

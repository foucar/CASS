// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __HISTOGRAM_GETTER_H__
#define __HISTOGRAM_GETTER_H__

#include <string>

#include "cass.h"
#include "postprocessing/postprocessor.h"

namespace cass
{

  /** Histogram retrievel parameters
  @author Jochen Küpper
  */
  struct HistogramParameter
  {
    HistogramParameter(PostProcessors::id_t _type)
      : type(_type)
    {};
    PostProcessors::id_t type;
  };



  //class that will retrive a histogram from the histogram container//
  class CASSSHARED_EXPORT HistogramGetter
  {
  public:

    HistogramGetter(const PostProcessors::histograms_t& histograms)
      : _histograms(histograms)
    {}
    //function that will serializethe requested histogram to a string and return it
    const std::string operator()(const HistogramParameter&) const;

  protected:
    const PostProcessors::histograms_t &_histograms;  //histogram container
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

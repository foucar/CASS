// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009,2010 Lutz Foucar

#ifndef CASS_CONVERSIONBACKEND_H
#define CASS_CONVERSIONBACKEND_H

#include "cass.h"
#include <stdint.h>
#include <vector>
#include <algorithm>

namespace Pds
{
  class Xtc;
}

namespace cass
{
  class CASSEvent;

  /** Base class for Converters
   *
   * Inherit from this class if you would like to add a new Converter
   *
   * @author Lutz Foucar
   * @author Jochen Kuepper
   */
  class CASSSHARED_EXPORT ConversionBackend
  {
  public:
    /** virtual destructor to make clear this is a base class*/
    virtual ~ConversionBackend() {}
    /** pure virtual operator call this to convert the xtc to the cass event*/
    virtual void operator()(const Pds::Xtc*, cass::CASSEvent*) = 0;
  };
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

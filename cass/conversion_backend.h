// Copyright (C) 2009 Jochen Küpper,lmf

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

  class CASSSHARED_EXPORT ConversionBackend
  {
  public:
    virtual ~ConversionBackend() {}
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

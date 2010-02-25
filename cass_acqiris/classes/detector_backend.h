// Copyright (C) 2010 lmf

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  class CASS_ACQIRISSHARED_EXPORT DetectorBackend
  {
  public:
    virtual ~DetectorBackend() {}
    enum DetectorTypes {DelaylineDetector, ToFDetector};
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

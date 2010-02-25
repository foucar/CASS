// Copyright (C) 2010 lmf

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT DetectorBackend
    {
    public:
      virtual ~DetectorBackend() {}
      DetectorAnalyzers    analyzerType()const    {return _analyzerType;}
      DetectorAnalyzers   &analyzerType()         {return _analyzerType;}
    protected:
      DetectorAnalyzers    _analyzerType;         //enum telling which analyzer should be used
    };
  }
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

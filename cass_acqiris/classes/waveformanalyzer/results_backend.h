//Copyright (C) Lutz Foucar

#ifndef _RESULTS_BACKEND_H_
#define _RESULTS_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! @brief Base class of Results
        used to retrun results of the waveform analysis
        @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT ResultsBackend
    {
    public:
      virtual ~ResultsBackend() {}
    };
  }//end namespace acqiris
}//end namespace cass
#endif

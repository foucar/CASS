//Copyright (C) 2010 Lutz Foucar

/**
 * @file results_backend.h file contains base class for all results of the
 *                         wavefrom analysis
 *
 * @author Lutz Foucar
 */

#ifndef _RESULTS_BACKEND_H_
#define _RESULTS_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Base class of Results.
     *
     * used to retrun results of the waveform analysis
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT ResultsBackend
    {
    public:
      virtual ~ResultsBackend() {}
    };
  }//end namespace acqiris
}//end namespace cass
#endif

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
  class CASSEvent;

  namespace ACQIRIS
  {
    /** Base class of Results.
     *
     * used to retrun results of the event analysis
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT ResultsBackend
    {
    public:
      virtual ~ResultsBackend() {}

      virtual void associate(const CASSEvent& evt)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

#ifndef _RESULTS_BACKEND_H_
#define _RESULTS_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT ResultsBackend
    {
    public:
      virtual ~ResultsBackend()         {}
    };
  }//end namespace remi
}//end namespace cass
#endif

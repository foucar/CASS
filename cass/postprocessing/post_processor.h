#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include "cass.h"

namespace cass
{
  class CASSEvent;

  class PostProcessor
  {
    public:
      PostProcessor(const char * outputfilename)   {}
      ~PostProcessor()  {}

    public:
      void postProcess(CASSEvent&);
  };
}


#endif

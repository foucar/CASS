#ifndef _CCD_DEVICE_H_
#define _CCD_DEVICE_H_

#include "cass_ccd.h"

namespace cass
{
  namespace ccd
  {
    class CASS_CCDSHARED_EXPORT CCDDevice : public cass::DeviceBackend
    {
    public:
      CCDDevice()   {}
      ~CCDDevice()  {}

    public:
      const CCDDetector &detector()const  {return _detector;}
      CCDDetector &detector()             {return _detector;}

    private:
      CCDDetector   _detector;
    }
  }
}

#endif

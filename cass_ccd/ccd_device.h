#ifndef _CCD_DEVICE_H_
#define _CCD_DEVICE_H_

#include "cass_ccd.h"
#include "device_backend.h"
#include "ccd_detector.h"

namespace cass
{
  namespace CCD
  {
    class CASS_CCDSHARED_EXPORT CCDDevice : public cass::DeviceBackend
    {
    public:
      CCDDevice()   {}
      ~CCDDevice()  {}

    public:
      const cass::CCDDetector &detector()const  {return _detector;}
      cass::CCDDetector &detector()             {return _detector;}

      void serialize(cass::Serializer&)const;
      void deserialize(cass::Serializer&);

    private:
      cass::CCDDetector   _detector;  //the ccd detector of this device
    };
  }
}

inline void cass::CCD::CCDDevice::serialize(cass::Serializer& out)const
{
//  _detector.serialize(out);
}
inline void cass::CCD::CCDDevice::deserialize(cass::Serializer& in)
{
//  _detector.serialize(in);
}

#endif

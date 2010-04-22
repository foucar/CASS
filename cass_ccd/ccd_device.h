//Copyright (C) 2010 lmf

#ifndef _CCD_DEVICE_H_
#define _CCD_DEVICE_H_

#include <iostream>
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
      CCDDevice()
        :DeviceBackend(1)
      {}
      ~CCDDevice()  {}

    public:
      const cass::CCDDetector &detector()const  {return _detector;}
      cass::CCDDetector &detector()             {return _detector;}

      void serialize(cass::Serializer&);
      void deserialize(cass::Serializer&);

    private:
      cass::CCDDetector   _detector;  //the ccd detector of this device
    };
  }
}


inline void cass::CCD::CCDDevice::serialize(cass::Serializer& out)
{
  //the version//
  out.addUint16(_version);
  //the detector//
  _detector.serialize(out);
}


inline void cass::CCD::CCDDevice::deserialize(cass::Serializer& in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in ccd: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //deserialize the detector
  _detector.deserialize(in);
}

#endif

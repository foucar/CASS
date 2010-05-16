//Copyright (C) 2010 Lutz Foucar

#ifndef _CCD_DEVICE_H_
#define _CCD_DEVICE_H_

#include <iostream>
#include "cass_ccd.h"
#include "device_backend.h"
//#include "ccd_detector.h"
#include "pixel_detector.h"

namespace cass
{
  namespace CCD
  {
    /** The commercial ccd device.
     * This device contains all comercial ccd detectors
     * @author Lutz Foucar
     */
    class CASS_CCDSHARED_EXPORT CCDDevice : public cass::DeviceBackend
    {
    public:
      CCDDevice()
        :DeviceBackend(1)
      {}
      ~CCDDevice()  {}
    public:
         typedef std::vector<PixelDetector> detectors_t;

    public:
      /*
      const cass::PixelDetector &detector()const  {return _detector;}
      cass::PixelDetector &detector()             {return _detector;}
      */
      void serialize(cass::SerializerBackend&);
      bool deserialize(cass::SerializerBackend&);

    public:
      const detectors_t *detectors()const   {return &_detectors;}
      detectors_t       *detectors()        {return &_detectors;}

    private:
      //cass::PixelDetector   _detector;  //the ccd detector of this device
      detectors_t   _detectors;  // !< a vector containing all pixel detectors
    };
  }
}


inline void cass::CCD::CCDDevice::serialize(cass::SerializerBackend& out)
{
  //the version//
  out.addUint16(_version);
  //serialize the amount of detectors present//
  size_t nDets = _detectors.size();
  out.addSizet(nDets);
  //serialize each detector//
  for (detectors_t::iterator it=_detectors.begin(); it != _detectors.end();++it)
    it->serialize(out);
}


inline bool cass::CCD::CCDDevice::deserialize(cass::SerializerBackend& in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in pnCCD: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  //read how many detectors//
  size_t nDets = in.retrieveSizet ();
  //make the detector container big enough//
  _detectors.resize(nDets);
  //deserialize each detector//
  for(detectors_t::iterator it=_detectors.begin(); it != _detectors.end(); ++it)
    it->deserialize(in);
  return true;
}

#endif

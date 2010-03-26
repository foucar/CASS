//Copyright (C) 2010 lmf, Nicola Coppola

#ifndef _PNCCD_DEVICE_H
#define _PNCCD_DEVICE_H

#define pnCCD_default_size    1024
#define pnCCD_default_size_sq 1024*1024

#include <iostream>
#include <vector>
#include <stdint.h>

#include "ccd_detector.h"
#include "cass_pnccd.h"
#include "device_backend.h"


namespace cass
{
  namespace pnCCD
  {
    class CASS_PNCCDSHARED_EXPORT pnCCDDevice : public DeviceBackend
    {
    public:
      pnCCDDevice(void)    {_version=1;}
      ~pnCCDDevice()       {}

    public:
      typedef std::vector<CCDDetector> detectors_t;

    public:
      void serialize(cass::Serializer&)const;
      void deserialize(cass::Serializer&);

    public:
      const detectors_t   &detectors()const   {return _detectors;}
      detectors_t         &detectors()        {return _detectors;}

    private:
      detectors_t          _detectors;        //a vector containing all ccd detectors
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

inline void cass::pnCCD::pnCCDDevice::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //serialize the amount of detectors present//
  size_t nDets = _detectors.size();
  out.addSizet(nDets);
  //serialize each detector//
  for (detectors_t::const_iterator it=_detectors.begin(); it != _detectors.end();++it)
    it->serialize(out);
}

inline void cass::pnCCD::pnCCDDevice::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in pnCCD: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //read how many detectors//
  size_t nDets = in.retrieveSizet ();
  //make the detector container big enough//
  _detectors.resize(nDets);
  //deserialize each detector//
  for(detectors_t::iterator it=_detectors.begin(); it != _detectors.end(); ++it)
    it->deserialize(in);
}

#endif // PNCCD_EVENT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.h



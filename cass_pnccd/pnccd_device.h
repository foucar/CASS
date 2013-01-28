//Copyright (C) 2010 Lutz Foucar
//Copyright (C) 2010 Nicola Coppola

#ifndef _PNCCD_DEVICE_H
#define _PNCCD_DEVICE_H

#include <iostream>
#include "cass_pnccd.h"
#include "device_backend.h"
#include "pixel_detector.h"

namespace cass
{
  namespace pnCCD
  {
    /** the pnccd device.
     *
     * The device contains a list of detectors and is serializable.
     *
     * @author Lutz Foucar
     */
    class CASS_PNCCDSHARED_EXPORT pnCCDDevice : public DeviceBackend
    {
    public:
      /** default constructor. defining the version*/
      pnCCDDevice()
        :DeviceBackend(1)
      {}

    public:
      /** serialize the device to the Serializer*/
      void serialize(cass::SerializerBackend&)const;
      /** deserialize the device from the Serializer*/
      bool deserialize(cass::SerializerBackend&);

    public:
      /** getter */
      const detectors_t   *detectors()const   {return &_detectors;}
      /** setter */
      detectors_t         *detectors()        {return &_detectors;}

    private:
      /** container for all pnccd detectors*/
      detectors_t          _detectors;
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

inline void cass::pnCCD::pnCCDDevice::serialize(cass::SerializerBackend &out)const
{
  writeVersion(out);
  //serialize the amount of detectors present//
  size_t nDets = _detectors.size();
  out.addSizet(nDets);
  //serialize each detector//
  for (detectors_t::const_iterator it=_detectors.begin(); it != _detectors.end();++it)
    it->serialize(out);
}

inline bool cass::pnCCD::pnCCDDevice::deserialize(cass::SerializerBackend &in)
{
  checkVersion(in);
  //read how many detectors//
  size_t nDets = in.retrieveSizet ();
  //make the detector container big enough//
  _detectors.resize(nDets);
  //deserialize each detector//
  for(detectors_t::iterator it=_detectors.begin(); it != _detectors.end(); ++it)
    it->deserialize(in);
  return true;
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



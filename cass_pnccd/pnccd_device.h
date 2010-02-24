#ifndef _PNCCD_DEVICE_H
#define _PNCCD_DEVICE_H


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
      pnCCDDevice(void)    {}
      ~pnCCDDevice()       {}

    public:
      typedef std::vector<CCDDetector> detectors_t;

    public:
      const detectors_t   &detectors()const   {return _detectors;}
      detectors_t         &detectors()        {return _detectors;}

    private:
      detectors_t          _detectors;        //a vector containing all ccd detectors
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass


#endif // PNCCD_EVENT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.h



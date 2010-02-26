#ifndef _ACQIRIS_DEVICE_H_
#define _ACQIRIS_DEVICE_H_

#include <vector>
#include "cass_acqiris.h"
#include "detector_backend.h"
#include "device_backend.h"
#include "channel.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT AcqirisDevice : public cass::DeviceBackend
    {
    public:
      AcqirisDevice()   {}
      ~AcqirisDevice()  {}

    public:
      typedef std::vector<Channel> channels_t;
      typedef std::vector<DetectorBackend> detectors_t;

    public:
      const channels_t   &channels()const         {return _channels;}
      channels_t         &channels()              {return _channels;}
      const detectors_t  &detectors()const        {return _dets;}
      detectors_t        &detectors()             {return _dets;}

    private:
      //containers for acqiris and delayline data//
      channels_t          _channels;              //Container for all Channels
      detectors_t         _dets;                  //Container for all Detektors
    };
  }//end namespace acqiris
}//end namespace cass
#endif

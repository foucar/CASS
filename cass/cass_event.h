#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <map>
#include <stdint.h>
#include "cass.h"


namespace cass
{
  class DeviceBackend;

  class CASSEvent
  {
  public:
    CASSEvent();
    ~CASSEvent();

  public:
    uint64_t             id()const        {return _id;}
    uint64_t            &id()             {return _id;}
    char                *datagrambuffer() {return _datagrambuffer;}
    const DeviceBackend &devices()const   {return _devices;}
    DeviceBackend       &devices()        {return _devices;}

  public:
    enum Device{pnCCD,Acqiris,Pulnix};
    typedef std::map<Device,DeviceBackend*> devices_t;

  private:
    uint64_t   _id;
    devices_t  _devices;
    char       _datagrambuffer[cass::DatagramBufferSize];
  };
}//end namespace

#endif // CASSEVENT_H

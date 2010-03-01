#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <map>
#include <stdint.h>
#include "cass.h"


namespace cass
{
  class DeviceBackend;

  class CASSSHARED_EXPORT CASSEvent
  {
  public:
    CASSEvent();
    ~CASSEvent();

  public:
    enum Device{pnCCD,Acqiris,ccd,MachineData};
    typedef std::map<Device,DeviceBackend*> devices_t;

  public:
    void serialize(bufferinputiterator_t&)const;
    void deserialize(bufferoutputiterator_t&);

  public:
    uint64_t         id()const        {return _id;}
    uint64_t        &id()             {return _id;}
    char            *datagrambuffer() {return _datagrambuffer;}
    const devices_t &devices()const   {return _devices;}
    devices_t       &devices()        {return _devices;}

  private:
    uint64_t   _id;         //id of the cassevent
    devices_t  _devices;    //list of devices for this event
    char       _datagrambuffer[cass::DatagramBufferSize]; //buffer for the datagram
  };
}//end namespace

#endif // CASSEVENT_H

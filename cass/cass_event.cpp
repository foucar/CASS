#include "cass_event.h"

#include "acqiris_device.h"
#include "ccd_device.h"
#include "pnccd_device.h"
#include "machine_device.h"


cass::CASSEvent::CASSEvent()
    :_id(0)
{
  //add all devices that are available
  _devices[ccd]         = new cass::CCD::CCDDevice();
  _devices[MachineData] = new cass::MachineData::MachineDataDevice();
  _devices[Acqiris]     = new cass::ACQIRIS::AcqirisDevice();
  _devices[pnCCD]       = new cass::pnCCD::pnCCDDevice();
}

cass::CASSEvent::~CASSEvent()
{
  //delete all devices
  for (devices_t::iterator it=_devices.begin() ; it != _devices.end(); ++it )
    delete (it->second);
}

void cass::CASSEvent::serialize(cass::Serializer& buffer)const
{
//  //serialize the id and then all devices//
//  std::copy( reinterpret_cast<const char*>(&_id),
//             reinterpret_cast<const char*>(&_id)+sizeof(uint64_t),
//             buffer);
//  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
//    it->second->serialize(buffer);
}

void cass::CASSEvent::deserialize(cass::Serializer& buffer)
{
  //get all infos from the buffer//
//  std::copy(buffer,
//            buffer+sizeof(uint64_t),
//            reinterpret_cast<char*>(&_id));
//  buffer += sizeof(uint64_t);
//  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
//    it->second->deserialize(buffer);
}

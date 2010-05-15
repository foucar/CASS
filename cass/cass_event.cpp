//Copyright (C) 2010 lmf

#include <iostream>

#include "cass_event.h"
#include "acqiris_device.h"
#include "ccd_device.h"
#include "pnccd_device.h"
#include "machine_device.h"


cass::CASSEvent::CASSEvent()
  :cass::Serializable(1),
  _id(0)
{
  //add all devices that are available
  _devices[CCD]         = new cass::CCD::CCDDevice();
  _devices[MachineData] = new cass::MachineData::MachineDataDevice();
  _devices[Acqiris]     = new cass::ACQIRIS::Device();
  _devices[pnCCD]       = new cass::pnCCD::pnCCDDevice();
}

cass::CASSEvent::~CASSEvent()
{
  //delete all devices
  for (devices_t::iterator it=_devices.begin() ; it != _devices.end(); ++it )
    delete (it->second);
}

void cass::CASSEvent::serialize(cass::SerializerBackend& out)
{
  //the version//
  out.addUint16(_version);

  //the id//
  out.addUint64(_id);

  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->serialize(out);
}

void cass::CASSEvent::deserialize(cass::SerializerBackend& in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in cass-event: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //get id//
  _id = in.retrieveUint64();
  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->deserialize(in);
}

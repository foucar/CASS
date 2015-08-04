//Copyright (C) 2010,2013 Lutz Foucar

/**
 * @file cass_event.cpp file contains defintion of the CASSEvent
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "cass_event.h"

#include "acqiris_device.hpp"
#include "acqiristdc_device.hpp"
#include "machine_device.hpp"
#include "pixeldetector.hpp"

using namespace cass;

CASSEvent::CASSEvent()
  : Serializable(1),
    _id(0),
    _datagrambuffer()
{
  //add all devices that are available
  _devices[MachineData]    = DeviceBackend::shared_pointer(new MachineData::Device());
  _devices[Acqiris]        = DeviceBackend::shared_pointer(new ACQIRIS::Device());
  _devices[AcqirisTDC]     = DeviceBackend::shared_pointer(new ACQIRISTDC::Device());
  _devices[PixelDetectors] = DeviceBackend::shared_pointer(new pixeldetector::Device());
}

void CASSEvent::serialize(SerializerBackend& out) const
{
  writeVersion(out);
  //the id//
  out.addUint64(_id);

  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->serialize(out);
}

bool CASSEvent::deserialize(SerializerBackend& in)
{
  checkVersion(in);
  //get id//
  _id = in.retrieveUint64();
  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->deserialize(in);
  return true;
}

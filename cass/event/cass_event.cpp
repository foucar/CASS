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
  /** write the id */
  out.add(_id);
  /** write the size of the container */
  out.add(static_cast<size_t>(_devices.size()));
  /** for each instrument in the map write the key and then the Instrument */
  for (devices_t::const_iterator it(_devices.begin()); it != _devices.end(); ++it)
  {
    out.add(it->first);
    it->second->serialize(out);
  }
}

bool CASSEvent::deserialize(SerializerBackend& in)
{
  checkVersion(in);
  _id = in.retrieve<uint64_t>();
  /** read the number of instruments */
  size_t nDev(in.retrieve<size_t>());
  /** read the key of the device and deserialize the right device from the list */
  for(size_t i(0); i < nDev; ++i)
  {
    const devices_t::key_type key(in.retrieve<devices_t::key_type>());
    _devices[key]->deserialize(in);
  }
  return true;
}

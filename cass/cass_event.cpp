//Copyright (C) 2010,2013 Lutz Foucar

/**
 * @file cass_event.cpp file contains defintion of the CASSEvent
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "cass_event.h"
#include "acqiris_device.h"
#include "acqiristdc_device.h"
#include "ccd_device.h"
#include "pnccd_device.h"
#include "machine_device.h"
#include "pixeldetector.hpp"

using namespace cass;

CASSEvent::CASSEvent()
  : cass::Serializable(1),
    _id(0),
    _datagrambuffer(cass::DatagramBufferSize,0)
{
  //add all devices that are available
  _devices[CCD]            = new cass::CCD::CCDDevice();
  _devices[MachineData]    = new cass::MachineData::MachineDataDevice();
  _devices[Acqiris]        = new cass::ACQIRIS::Device();
  _devices[AcqirisTDC]     = new cass::ACQIRISTDC::Device();
  _devices[pnCCD]          = new cass::pnCCD::pnCCDDevice();
  _devices[PixelDetectors] = new cass::pixeldetector::Device();
}

CASSEvent::~CASSEvent()
{
  //delete all devices
  for (devices_t::iterator it=_devices.begin() ; it != _devices.end(); ++it )
    delete (it->second);
}

void CASSEvent::serialize(cass::SerializerBackend& out)const
{
  writeVersion(out);
  //the id//
  out.addUint64(_id);

  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->serialize(out);
}

bool CASSEvent::deserialize(cass::SerializerBackend& in)
{
  checkVersion(in);
  //get id//
  _id = in.retrieveUint64();
  //all devices//
  for (devices_t::const_iterator it=_devices.begin(); it != _devices.end() ;++it)
    it->second->deserialize(in);
  return true;
}

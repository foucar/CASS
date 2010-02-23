#include "cass_event.h"

#include "remi_event.h"
#include "ccd_device.h"
#include "pnccd_event.h"
#include "machine_event.h"


cass::CASSEvent::CASSEvent():
    _id(0),
{
  //add all devices that are available
  _devices[Pulnix] = new cass::CCD::CCDDevice();
}

cass::CASSEvent::~CASSEvent()
{
  //delete all devices
  for (device_t::iterator it=_devices.begin() ; it != _devices.end(); ++it )
    delete (it->second);
}

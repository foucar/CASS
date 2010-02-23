#include "cass_event.h"

#include "remi_event.h"
#include "vmi_event.h"
#include "pnccd_event.h"
#include "machine_event.h"


cass::CASSEvent::CASSEvent():
    _id(0),
    _remievent(new REMI::REMIEvent()),
    _vmievent(new VMI::VMIEvent()),
    _pnccdevent(new pnCCD::pnCCDEvent()),
    _machinedataevent(new MachineData::MachineDataEvent())
{
}

cass::CASSEvent::~CASSEvent()
{
  delete _machinedataevent;
  delete _pnccdevent;
  delete _vmievent;
  delete _remievent;
}

#include "cass_event.h"

#include "remi_event.h"
#include "vmi_event.h"
#include "pnccd_event.h"

//ClassImp(cass::CASSEvent)

cass::CASSEvent::CASSEvent(uint64_t id):
        _id(id),
        _remievent(new REMI::REMIEvent()),
        _vmievent(new VMI::VMIEvent()),
	_pnccdevent(new pnCCD::pnCCDEvent())
{
}

cass::CASSEvent::~CASSEvent()
{
    delete _pnccdevent;
    delete _vmievent;
    delete _remievent;
}

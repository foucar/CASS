#include "cassevent.h"

#include "REMIEvent.h"
#include "VMIEvent.h"

cass::CASSEvent::CASSEvent(uint64_t id):
        _id(id),
        _remievent(new REMI::REMIEvent()),
        _vmievent(new VMI::VMIEvent())
{
}

cass::CASSEvent::~CASSEvent()
{
    delete _vmievent;
    delete _remievent;
}

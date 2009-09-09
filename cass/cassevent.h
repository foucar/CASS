#ifndef CASSEVENT_H
#define CASSEVENT_H

#include "REMIEvent.h"
#include "VMIEvent.h"

namespace cass
{
    class CASSEvent
    {
    public:
        CASSEvent() {}

        REMI::REMIEvent &REMIEvent()        {return _remievent;}
        VMI::VMIEvent   &VMIEvent()         {return _vmievent;}


    private:
        REMI::REMIEvent     _remievent;
        VMI::VMIEvent       _vmievent;
//        pnCCDEvent      _pnccdevent;
//        MACHINEEvent    _machineevent;
    };
}

#endif // CASSEVENT_H

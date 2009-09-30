#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <stdint.h>

//#ifndef ROOT_Rtypes
//#include "Rtypes.h"
//#endif


namespace cass
{
    namespace REMI
    {
        class REMIEvent;
    }
    namespace VMI
    {
        class VMIEvent;
    }
    namespace pnCCD
    {
        class pnCCDEvent;
    }

    class CASSEvent
    {
    public:
        CASSEvent(uint64_t id);
        ~CASSEvent();

        uint64_t         id()const          {return _id;}

        REMI::REMIEvent     &REMIEvent()    {return *_remievent;}
        VMI::VMIEvent       &VMIEvent()     {return *_vmievent;}
        pnCCD::pnCCDEvent   &pnCCDEvent()   {return *_pnccdevent;}

        //ClassDef(CASSEvent,1)

    private:
        uint64_t            _id;
        REMI::REMIEvent    *_remievent;
        VMI::VMIEvent      *_vmievent;
        pnCCD::pnCCDEvent  *_pnccdevent;
//        MACHINEEvent   *_machineevent;
    };
}

#endif // CASSEVENT_H

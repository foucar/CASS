#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <stdint.h>

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

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
    namespace MachineData
    {
        class MachineDataEvent;
    }

    class CASSEvent
    {
    public:
        CASSEvent()     {}
        CASSEvent(uint64_t id);
        ~CASSEvent();

    public:
        uint64_t    id()const   {return _id;}

    public:
        REMI::REMIEvent                 &REMIEvent()          {return *_remievent;}
        VMI::VMIEvent                   &VMIEvent()           {return *_vmievent;}
        pnCCD::pnCCDEvent               &pnCCDEvent()         {return *_pnccdevent;}
        MachineData::MachineDataEvent   &MachineDataEvent()   {return *_machinedataevent;}

    private:
        uint64_t                         _id;
        REMI::REMIEvent                 *_remievent;
        VMI::VMIEvent                   *_vmievent;
        pnCCD::pnCCDEvent               *_pnccdevent;
        MachineData::MachineDataEvent   *_machinedataevent;

	ClassDefNV(CASSEvent,1);
    };
}

#endif // CASSEVENT_H

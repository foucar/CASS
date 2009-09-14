#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <stdint.h>

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

    class CASSEvent
    {
    public:
        CASSEvent(uint64_t id);
        ~CASSEvent();

        uint64_t         id()const          {return _id;}

        REMI::REMIEvent &REMIEvent()        {return *_remievent;}
        VMI::VMIEvent   &VMIEvent()         {return *_vmievent;}


    private:
        uint64_t            _id;
        REMI::REMIEvent    *_remievent;
        VMI::VMIEvent      *_vmievent;
//        pnCCDEvent     *_pnccdevent;
//        MACHINEEvent   *_machineevent;
    };
}

#endif // CASSEVENT_H

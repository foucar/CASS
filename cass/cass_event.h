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
      CASSEvent();
      ~CASSEvent();

    public:
      uint64_t    id()const   {return _id;}
      uint64_t   &id()        {return _id;}
        
    public:
      char                            *datagrambuffer()     {return _datagrambuffer;} 

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
      char                             _datagrambuffer[0x1000000];
  };
}

#endif // CASSEVENT_H

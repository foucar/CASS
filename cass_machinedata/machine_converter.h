#ifndef MACHINEDATACONVERTER_H
#define MACHINEDATACONVERTER_H

#include <map>
#include "cass_machine.h"
#include "conversion_backend.h"
#include "machine_device.h"

namespace cass
{
  class CASSEvent;

  namespace MachineData
  {
    class CASS_MACHINEDATASHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      Converter();
      //called for appropriate xtc part//
      void operator()(const Pds::Xtc*, cass::CASSEvent*);

    private:
      typedef std::map<int,std::string> indexMap_t;

    private:
      IndexMap          _index2name;  //map to convert indexes to strings
      MachineDataDevice _store;       //a container for the machindata values
                                      //this is necessary, since not every shot there
                                      //is info about the epics values
    };
  }//end namespace MachineData
}//end namespace cass

#endif

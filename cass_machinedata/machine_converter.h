#ifndef MACHINEDATACONVERTER_H
#define MACHINEDATACONVERTER_H

#include <map>
#include "cass_machine.h"
#include "conversion_backend.h"
#include "machine_event.h"

namespace cass
{
    class CASSEvent;

    namespace MachineData
    {
        class CASS_MACHINEDATASHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            Converter();
            //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);

        private:
            typedef std::map<int,std::string> IndexMap;
            IndexMap            _index2name;
            MachineDataEvent    _storedevent;


        };
    }//end namespace MachineData
}//end namespace cass

#endif

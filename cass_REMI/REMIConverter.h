#ifndef REMICONVERTER_H
#define REMICONVERTER_H

#include "cass_remi.h"
#include "ConversionBackend.h"
#include "REMIEvent.h"

namespace cass
{
    namespace REMI
    {
        class CASS_REMISHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
           //called for LCLS event//
//            void operator()(const Pds::Acqiris::ConfigV1&, REMIEvent&);
//            void operator()(const Pds::Acqiris::DataDescV1&, REMIEvent&);
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
        private:
            //store the config internally since its only send once for each run//
            REMIEvent _storedEvent;
        };
    }//end namespace remi
}//end namespace cass

#endif

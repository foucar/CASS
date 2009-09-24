#ifndef REMICONVERTER_H
#define REMICONVERTER_H

#include "cass_remi.h"
#include "conversion_backend.h"
#include "remi_event.h"
#include <iostream>

namespace cass
{
    namespace REMI
    {
        class CASS_REMISHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
           //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
        private:
            //store the config internally since its only send once for each run//
            REMIEvent _storedEvent;
        };
    }//end namespace remi
}//end namespace cass

#endif

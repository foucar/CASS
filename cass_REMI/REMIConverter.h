#ifndef REMICONVERTER_H
#define REMICONVERTER_H

#include "cass_REMI.h"
#include "ConversionBackend.h"
#include "REMIEvent.h"

namespace cass
{
    namespace REMI
    {
        class CASS_REMISHARED_EXPORT Converter : cass::ConversionBackend
        {
        public:
           //called for LCLS event//
            void operator()(const Pds::Acqiris::ConfigV1&, const Pds::Acqiris::DataDescV1&, REMIEvent&);

        };
    }//end namespace remi
}//end namespace cass

#endif

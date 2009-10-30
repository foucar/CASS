#ifndef VMICONVERTER_H
#define VMICONVERTER_H

#include "cass_vmi.h"
#include "conversion_backend.h"

namespace cass
{
    class CASSEvent;

    namespace VMI
    {
        class CASS_VMISHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            Converter();
            //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
            bool handlesType(uint16_t type);
        };
    }//end namespace vmi
}//end namespace cass

#endif

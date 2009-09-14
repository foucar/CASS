#ifndef VMICONVERTER_H
#define VMICONVERTER_H

#include "cass_VMI.h"
#include "ConversionBackend.h"

namespace cass
{
    class CASSEvent;

    namespace VMI
    {
        class CASS_VMISHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);

        };
    }//end namespace vmi
}//end namespace cass

#endif

#ifndef VMICONVERTER_H
#define VMICONVERTER_H

#include "cass_VMI.h"
#include "VMIEvent.h"
#include "ConversionBackend.h"

namespace cass
{
    namespace VMI
    {
        class CASS_VMISHARED_EXPORT Converter : cass::ConversionBackend
        {
        public:
           //called for LCLS event//
            void operator()(const Pds::Camera::FrameV1&, VMIEvent&);

        };
    }//end namespace vmi
}//end namespace cass

#endif

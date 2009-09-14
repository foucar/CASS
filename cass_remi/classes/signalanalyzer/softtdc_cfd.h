#ifndef __SoftTDCCFD_h__
#define __SoftTDCCFD_h__

#include "softtdc.h"

namespace cass
{
    namespace REMI
    {
        //this is called in case it is a 8 Bit Instrument
        class SoftTDCCFD8Bit : public SoftTDC
        {
        public:
            void FindPeaksIn(REMIEvent&);
        };

        //this is called in case it is a 10 Bit Instrument
        class SoftTDCCFD16Bit : public SoftTDC
        {
        public:
            void FindPeaksIn(REMIEvent&);
        };

    }//end namespace remi
}//end namespace cass
#endif

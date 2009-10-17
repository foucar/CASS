#ifndef PNCCDCONVERTER_H
#define PNCCDCONVERTER_H

#include "pdsdata/pnCCD/fformat.h"
#include "cass_pnccd.h"
#include "pnccd_event.h"
#include "conversion_backend.h"

namespace Pds
{
    class Xtc;
}

namespace cass
{
    class CASSEvent;
    namespace pnCCD
    {
        class CASS_PNCCDSHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
        private:
            pnCCDEvent _storedEvent;

        };
    }//end namespace vmi
}//end namespace cass

#endif

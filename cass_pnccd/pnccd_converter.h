#ifndef PNCCDCONVERTER_H
#define PNCCDCONVERTER_H

#include "cass_pnccd.h"
//#include "pnccd_event.h"
#include "conversion_backend.h"

namespace Pds
{
    namespace PNCCD
    {
        class ConfigV1;
    }
}
namespace cass
{
  class CASSEvent;
    namespace pnCCD
    {
        class CASS_PNCCDSHARED_EXPORT Converter : public cass::ConversionBackend
        {
        public:
            Converter();
            //called for LCLS event//
            void operator()(const Pds::Xtc*, cass::CASSEvent*);
        private:
            Pds::PNCCD::ConfigV1 *_pnccdConfig;

        };
    }//end namespace vmi
}//end namespace cass

#endif

#ifndef _CCD_CONVERTER_H_
#define _CCD_CONVERTER_H_

#include "cass_ccd.h"
#include "conversion_backend.h"

namespace cass
{
  class CASSEvent;

  namespace CCD
  {
    class CASS_CCDSHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      Converter() {}
      //called for LCLS event//
      void operator()(const Pds::Xtc*, cass::CASSEvent*);
    };
  }//end namespace ccd
}//end namespace cass

#endif

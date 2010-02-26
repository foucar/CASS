#ifndef _ACQIRIS_CONVERTER_H
#define _ACQIRIS_CONVERTER_H

#include "cass_acqiris.h"
#include "conversion_backend.h"
#include "acqiris_device.h"
#include <iostream>


namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      Converter():_numberOfChannels(0) {}
      void operator()(const Pds::Xtc*, cass::CASSEvent*);

    private:
      size_t _numberOfChannels;   //the number of channels for the device is only send once
    };
  }//end namespace acqiris
}//end namespace cass

#endif

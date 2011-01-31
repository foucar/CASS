//Copyright (C) 2011 Lutz Foucar

/**
 * @file acqiristdc_converter.h file contains the declaration of the converter
 *                              for the xtc containing acqiris tdc data.
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRISTDC_CONVERTER_H
#define _ACQIRISTDC_CONVERTER_H

#include <iostream>
#include <map>

#include "cass_acqiris.h"
#include "conversion_backend.h"
#include "acqiris_device.h"


namespace cass
{
  namespace ACQIRISTDC
  {
    /** Acqiris Converter
     *
     * this class takes a xtc of type Id_AcqTDC and extracts the acqiris tdc
     * channels for all instruments
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      /** constructor
       *
       * sets up the pds type ids that it is responsible for
       */
      Converter();

      /** takes the xtc and copies the data to cassevent */
      void operator()(const Pds::Xtc*, cass::CASSEvent*);
    };
  }//end namespace acqiris
}//end namespace cass

#endif

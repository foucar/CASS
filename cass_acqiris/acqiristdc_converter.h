//Copyright (C) 2011 Lutz Foucar

/**
 * @file acqiristdc_converter.h file contains the declaration of the converter
 *                              for the xtc containing acqiris tdc data.
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRISTDC_CONVERTER_H
#define _ACQIRISTDC_CONVERTER_H

#include <QtCore/QMutex>

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
      /** create singleton if doesnt exist already */
      static ConversionBackend::converterPtr_t instance();

      /** takes the xtc and copies the data to cassevent */
      void operator()(const Pds::Xtc*, cass::CASSEvent*);

    private:
      /** constructor
       *
       * sets up the pds type ids that it is responsible for
       */
      Converter();

      /** prevent copy construction */
      Converter(const Converter&);

      /** prevent assignment */
      Converter& operator=(const Converter&);

      /** the singleton container */
      static ConversionBackend::converterPtr_t _instance;

      /** singleton locker for mutithreaded requests */
      static QMutex _mutex;
    };
  }//end namespace acqiris
}//end namespace cass

#endif

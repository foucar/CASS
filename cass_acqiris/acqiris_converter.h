//Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file acqiris_converter.h file contains the declaration of the converter
 *                           for the xtc containing acqiris data.
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRIS_CONVERTER_H
#define _ACQIRIS_CONVERTER_H

#include <iostream>
#include <map>

#include <QtCore/QMutex>

#include "cass_acqiris.h"
#include "conversion_backend.h"
#include "acqiris_device.h"


namespace cass
{
  namespace ACQIRIS
  {
    /** Acqiris Converter
     *
     * this class takes a xtc of type Id_AcqWaveform or Id_AcqConfig and
     * extracts the acqiris channels for all instruments
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
       * sets up the pds type ids it is responsible for
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

      /** Number of Channels for a device
       *
       * the number of channels for the device is only send with a configure
       * transition we store them in a map for each instrument
       */
      std::map<Instruments,size_t> _numberOfChannels;
    };
  }//end namespace acqiris
}//end namespace cass

#endif

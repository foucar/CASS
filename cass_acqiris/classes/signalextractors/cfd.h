//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file cfd.h file contains declaration of class that does a constant fraction
 *             descrimination like analysis of a waveform
 *
 * @author Lutz Foucar
 */

#ifndef _CFD_H_
#define _CFD_H_

#include <iostream>
#include "cass_acqiris.h"
#include "cass.h"
#include "signal_extractor.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Finds Signals in a waveform.
     *
     * Analyzes a waveform and find signals using a constant fraction algorithm.
     * It then does all the further analysis of the identified Signal.
     *
     * This class will work on waveforms of old 8 Bit Acqiris Instruments.
     *
     * @note we should let this class only identify the Signals and create the
     *       Signal list. The further analysis of the Signal should be done,
     *       when the user requests a property.
     * @note we might be able to not have this a template by using
     *       waveform_t::value_type..
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public SignalExtractor
    {
    public:
      /** constructor*/
      CFD8Bit()    {VERBOSEOUT(std::cout << "adding 8 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      /** extract signals form the CASSEvent
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor */
      void loadSettings(CASSSettings&);
     };

    /** Finds signals in a 16 bit waveform.
     *
     * @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public SignalExtractor
    {
    public:
      CFD16Bit()    {VERBOSEOUT(std::cout << "adding 16 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor */
      void loadSettings(CASSSettings&);
    };

  }//end namespace acqiris
}//end namespace cass
#endif

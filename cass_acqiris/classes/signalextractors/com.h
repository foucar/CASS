// Copyright (C) 2003 - 2010 Lutz Foucar

/**
 * @file com.h file contains declaration of class that does a center of mass
 *             analysis of a waveform
 *
 * @author Lutz Foucar
 */

#ifndef __COM_H__
#define __COM_H__

#include <iostream>
#include "cass.h"
#include "cass_acqiris.h"
#include "signal_extractor.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Finds Signals in a waveform.
     *
     * Analyzes a waveform and find signals when they have three consecutive
     * points above the defined threshold. It then does all the further analysis
     * of the identified Signal.
     *
     * This class will work on waveforms of old 8 Bit Acqiris Instruments.
     *
     * @note we should let this class only identify the Signals and create the
     *       Signal list. The further analysis of the Signal should be done,
     *       when the user requests a property.
     * @note when the above is done we need to rename this to something
     *       that describes the functors class much better.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM8Bit : public SignalExtractor
    {
    public:
      /** constructor*/
      CoM8Bit()    {VERBOSEOUT(std::cout << "adding 8 bit Center of Mass waveformanalyzer"<<std::endl);}

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

    /** Finds singals in a 16 bit waveform.
     *
     * @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM16Bit : public SignalExtractor
    {
    public:
      CoM16Bit()    {VERBOSEOUT(std::cout << "adding 16 bit Center of Mass waveformanalyzer"<<std::endl);}

      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor */
      void loadSettings(CASSSettings&);
    };
  }//end namespace acqiris
}//end namespace cass

#endif

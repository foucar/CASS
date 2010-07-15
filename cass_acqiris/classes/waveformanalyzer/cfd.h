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
#include "waveform_analyzer_backend.h"

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
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public WaveformAnalyzerBackend
    {
    public:
      /** constructor*/
      CFD8Bit()    {VERBOSEOUT(std::cout << "adding 8 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      /** analzye the waveform of the channel.
       *
       * @return void
       * @param c The channel that we need to analyze
       * @param r The found peaks go to the result
       */
      virtual void operator()(const Channel&c, ResultsBackend&r);
    };

    /** Finds signals in a 16 bit waveform.
     *
     * @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public WaveformAnalyzerBackend
    {
    public:
      CFD16Bit()    {VERBOSEOUT(std::cout << "adding 16 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      virtual void operator()(const Channel&, ResultsBackend&);
    };

  }//end namespace acqiris
}//end namespace cass
#endif

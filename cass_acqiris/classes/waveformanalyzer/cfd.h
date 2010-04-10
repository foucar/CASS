//Copyright (C) 2009,2010 Lutz Foucar

#ifndef _CFD_H_
#define _CFD_H_

#include <iostream>
#include "cass_acqiris.h"
#include "waveform_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! @brief Finds Signals in a waveform.
       Analyzes a waveform and find signals using a constant fraction algorithm.
       It then does all the further analysis of the identified Signal.
       This class will work on waveforms of old 8 Bit Acqiris Instruments.
       @todo we should let this class only identify the Signals and create the
             Signal list. The further analysis of the Signal should be done,
             when the user requests a property.
    @author Lutz Foucar
  */
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public WaveformAnalyzerBackend
    {
    public:
      /** constructor*/
      CFD8Bit()    {std::cout << "adding 8 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl;}
      /** analzye the waveform of the channel.
        @return void
        @param c The channel that we need to analyze
        @param r The found peaks go to the result
      */
      void analyze(const Channel&c, ResultsBackend&r);
    };

    /*! @brief Finds signals in a 16 bit waveform
        @see class CoM8Bit
        @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public WaveformAnalyzerBackend
    {
    public:
      CFD16Bit()    {std::cout << "adding 16 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl;}
      void analyze(const Channel&, ResultsBackend&);
    };

  }//end namespace acqiris
}//end namespace cass
#endif

// Copyright (C) 2003 - 2010 Lutz Foucar
#ifndef __COM_H__
#define __COM_H__

#include <iostream>
#include "cass_acqiris.h"
#include "waveform_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Finds Signals in a waveform.
     * Analyzes a waveform and find signals when they have three consecutive
     * points above the defined threshold. It then does all the further analysis
     * of the identified Signal.
     * This class will work on waveforms of old 8 Bit Acqiris Instruments.
     * @note we should let this class only identify the Signals and create the
     *       Signal list. The further analysis of the Signal should be done,
     *       when the user requests a property.
     * @note when the above is done we need to rename this to something
     *       that describes the functors class much better.
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM8Bit : public WaveformAnalyzerBackend
    {
    public:
      /** constructor*/
      CoM8Bit()    {std::cout << "adding 8 bit Center of Mass waveformanalyzer"<<std::endl;}
      /** the actual functor that does all the work.
       * @return void
       * @param c the channel to work on
       * @param r the waveform signal that contains the peak list, that will be filley by this
       */
      virtual void operator()(const Channel&c, ResultsBackend&r);
    };

    /** Finds singals in a 16 bit waveform.
     * @see class CoM8Bit
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM16Bit : public WaveformAnalyzerBackend
    {
    public:
      CoM16Bit()    {std::cout << "adding 16 bit Center of Mass waveformanalyzer"<<std::endl;}
      virtual void operator()(const Channel&, ResultsBackend&);
    };
  }//end namespace acqiris
}//end namespace cass

#endif

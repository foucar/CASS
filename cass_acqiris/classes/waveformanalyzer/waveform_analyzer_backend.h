//Copyright (C) 2010 Lutz Foucar

#ifndef _WAVEFORM_ANALYZER_BACKEND_H_
#define _WAVEFORM_ANALYZER_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    //forward declarations
    class Channel;
    class ResultsBackend;

    /** Base class for Wavefrom analyzers
     *
     * waveform analyzers should take a channel, analyze its wavefrom
     * and put the result of the analysis into the results base class
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT WaveformAnalyzerBackend
    {
    public:
      /** virtual destructor*/
      virtual ~WaveformAnalyzerBackend(){}

      /** pure virtual function stub for all analyzers that analyze a waveform
       *
       * @return void
       * @param[in] c The channel to work on
       * @param[out] r the results of the analysis go here
       */
      virtual void operator()(const Channel& c, ResultsBackend& r) = 0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

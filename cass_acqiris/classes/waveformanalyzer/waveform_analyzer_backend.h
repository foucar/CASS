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
    /*! @brief Base class for Wavefrom analyzers
      waveform analyzers should take a channel, analyze its wavefrom
      and put the result of the analysis into the results base class
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT WaveformAnalyzerBackend
    {
    public:
      /** virtual destructor*/
      virtual ~WaveformAnalyzerBackend(){}
      /** pure virtual function stub for all analyzers that analyze a waveform
        @todo to make this more clearly a function this should be renamed to operator ()
        */
      virtual void analyze(const Channel&, ResultsBackend&) = 0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

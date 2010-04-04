//Copyright (C) 2010 Lutz Foucar

#ifndef __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_
#define __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_

#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! @brief base class for delayline detector analysis
      @todo since there is more than 1 way to rekonstruct the detectorhits
            we might still need this base class
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerBackend
        : public DetectorAnalyzerBackend
    {
    public:
      /** constructor
        @todo does this need to know what kind of waveform analyzers are present?
              the list of Singals / Peak should be created by the class Signal itselve?*/
      DelaylineDetectorAnalyzerBackend(waveformanalyzers_t* waveformanalyzer)
          :DetectorAnalyzerBackend(waveformanalyzer) {}
      /** virtual destructor*/
      virtual ~DelaylineDetectorAnalyzerBackend() {}
      /** create the list of detector hits
        @todo to make more clear that this is a function we should rename this to
              operator ()
        @todo instead of the channel list, we have to give the whole device, since
              a detector only knows in which instrument the channels are in*/
      virtual void analyze(DetectorBackend&,const std::vector<Channel>&)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

#ifndef __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_
#define __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_

#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    //base class for delayline detector analysis
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerBackend
        : public DetectorAnalyzerBackend
    {
    public:
      DelaylineDetectorAnalyzerBackend(waveformanalyzers_t* waveformanalyzer)
          :DetectorAnalyzerBackend(waveformanalyzer) {}
      virtual ~DelaylineDetectorAnalyzerBackend() {}
      virtual void analyze(DetectorBackend&,const std::vector<Channel>&)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

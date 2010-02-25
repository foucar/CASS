#ifndef __DELAYLINE_DETECTOR_ANALYZER_H_
#define __DELAYLINE_DETECTOR_ANALYZER_H_

#include <vector>
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
      DelaylineDetectorAnalyzer(waveformanalyzers_t* waveformanalyzer)
          :DetectorAnalyzerBackend(waveformanalyzer) {}
      virtual ~DelaylineDetectorAnalyzer() {}
      virtual void analyze(DetectorBackend&,const std::vector<Channel>&)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

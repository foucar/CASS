#ifndef __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_
#define __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_

#include "delayline_detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    class DelaylineDetector;
    class AnodeLayer;

    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerSimple
        : public DelaylineDetectorAnalyzerBackend
    {
    public:
      DetectorHitSorterSimple()
          :DelaylineDetectorAnalyzerBackend(waveformanalyzer)
      {
        std::cout << "adding simple delayline detector analyzer"<<std::endl;
      }
      void analyze(DetectorBackend&,const std::vector<Channel>&);
    private:
      void sortForTimesum(DelaylineDetector&,AnodeLayer &x,AnodeLayer &y);
    };

  }//end namespace acqiris
}//end namespace cass

#endif

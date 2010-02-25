#ifndef __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_
#define __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_

#include "delayline_detector_analyzer.h"

namespace cass
{
  namespace ACQIRIS
  {
    class DelaylineDetectorAnalyzerSimple : public DelaylineDetectorAnalyzer
    {
    public:
      DetectorHitSorterSimple()
          :DelaylineDetectorAnalyzer(waveformanalyzer){std::cout << "adding simple detectorhitsorter"<<std::endl;}
      void analyze(DetectorBackend&,const std::vector<Channel>&);
    public:
      enum LayersToUse {UV=0,UW,VW};

    private:
      void sortForTimesum(Detector&,AnodeLayer &x,AnodeLayer &y);
    };

  }//end namespace acqiris
}//end namespace cass

#endif

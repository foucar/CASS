// Copyright (C) 2010 lmf

#ifndef _DETECTOR_ANALYZER_BACKEND_H_
#define _DETECTOR_ANALYZER_BACKEND_H_

#include "vector"
#include "cass_acqiris.h"
#include "channel.h"

namespace cass
{
  namespace ACQIRIS
  {
    class DetectorBackend;
    class WaveformAnalyzerBackend;

    class CASS_ACQIRISSHARED_EXPORT DetectorAnalyzerBackend
    {
    public:
      DetectorAnalyzerBackend(waveformanalyzers_t* waveformanalyzer)
          :_waveformanalyzer(waveformanalyzer) {}
      virtual ~DetectorAnalyzerBackend() {}
      virtual void analyze(DetectorBackend&,const std::vector<Channel>&)=0;
    protected:
      typedef std::map<WaveformAnalyzers, WaveformAnalyzerBackend*> waveformanalyzers_t;
    protected:
      waveformanalyzers_t *_waveformanalyzer;
    };
  }
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

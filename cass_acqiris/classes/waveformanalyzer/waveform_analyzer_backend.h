#ifndef _WAVEFORM_ANALYZER_BACKEND_H_
#define _WAVEFORM_ANALYZER_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    class Channel;
    class ResultsBackend;
    class CASS_ACQIRISSHARED_EXPORT WaveformAnalyzerBackend
    {
    public:
      virtual ~WaveformAnalyzerBackend()         {}
      virtual void analyze(const Channel&, ResultsBackend&) = 0;
    };
  }//end namespace remi
}//end namespace cass
#endif

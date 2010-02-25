#ifndef _WAVEFORM_ANALYZER_BACKEND_H_
#define _WAVEFORM_ANALYZER_BACKEND_H_

#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    class Channel;
    class Signal;
    class CASS_ACQIRISSHARED_EXPORT WaveformAnalyzerBackend
    {
    public:
      virtual ~WaveformAnalyzerBackend()         {}
      virtual void analyze(const Channel&, Signal&) = 0;
    public:
      enum WaveformAnalyzerTypes{CoM8Bit,CoM16Bit,CFD8Bit,CFD16Bit};
    };
  }//end namespace remi
}//end namespace cass
#endif

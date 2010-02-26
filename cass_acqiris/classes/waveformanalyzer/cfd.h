#ifndef _CFD_H_
#define _CFD_H_

#include <iostream>
#include "cass_acqiris.h"
#include "waveform_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    //this is called in case it is a 8 Bit Instrument
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public WaveformAnalyzerBackend
    {
    public:
      CFD8Bit()    {std::cout << "adding 8 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl;}
      void analyze(const Channel&, ResultsBackend&);
    };

    //this is called in case it is a 10 Bit Instrument
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public WaveformAnalyzerBackend
    {
    public:
      CFD16Bit()    {std::cout << "adding 16 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl;}
      void analyze(const Channel&, ResultsBackend&);
    };

  }//end namespace acqiris
}//end namespace cass
#endif

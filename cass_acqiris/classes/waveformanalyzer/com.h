#ifndef __COM_H__
#define __COM_H__

#include <iostream>
#include "cass_acqiris.h"
#include "waveform_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    //this is called in case it is a 8 Bit Instrument
    class CASS_ACQIRISSHARED_EXPORT CoM8Bit : public WaveformAnalyzer
    {
    public:
      CoM8Bit()    {std::cout << "adding 8 bit Center of Mass waveformanalyzer"<<std::endl;}
      void analyze(const Channel&, ResultsBackend&);
    };

    //this is called in case it is a 10 Bit Instrument
    class CASS_ACQIRISSHARED_EXPORT CoM16Bit : public WaveformAnalyzer
    {
    public:
      CoM16Bit()    {std::cout << "adding 16 bit Center of Mass waveformanalyzer"<<std::endl;}
      void analyze(const Channel&, ResultsBackend&);
    };
  }//end namespace acqiris
}//end namespace cass

#endif

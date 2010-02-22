#ifndef __CoM_h__
#define __CoM_h__

#include "waveform_analyzer.h"
#include <iostream>

namespace cass
{
  namespace REMI
  {
    //this is called in case it is a 8 Bit Instrument
    class CoM8Bit : public WaveformAnalyzer
    {
    public:
      CoM8Bit()    {std::cout << "adding 8 bit Center of Mass waveformanalyzer"<<std::endl;}
      void analyze(Channel&, const double SampleInterval);
    };

    //this is called in case it is a 10 Bit Instrument
    class CoM16Bit : public WaveformAnalyzer
    {
    public:
      CoM16Bit()    {std::cout << "adding 16 bit Center of Mass waveformanalyzer"<<std::endl;}
      void analyze(Channel&, const double SampleInterval);
    };
  }//end namespace remi
}//end namespace cass

#endif

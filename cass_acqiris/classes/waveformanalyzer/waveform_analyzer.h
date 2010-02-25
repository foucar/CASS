#ifndef __WAVEFORMANALYZER_h__
#define __WAVEFORMANALYZER_h__

#include <iostream>

namespace cass
{
  namespace REMI
  {
    class Channel;
    //this class is placeholder for two other classes wich will be called
    //according to how many bits the instrument has
    class WaveformAnalyzer
    {
    public:
      virtual ~WaveformAnalyzer()         {}
      virtual void analyze(Channel&, const double SampleInterval) = 0;
    public:
      enum WaveformAnalyzerTypes{DoNothing,CoM8Bit,CoM16Bit,CFD8Bit,CFD16Bit};
    };

    class WaveformAnalyzerDoNothing : public WaveformAnalyzer
    {
    public:
      WaveformAnalyzerDoNothing(){std::cout<<"adding nothing waveformanalyzer"<<std::endl;}
      void analyze(Channel&, const double SampleInterval) {}
    };

  }//end namespace remi
}//end namespace cass
#endif

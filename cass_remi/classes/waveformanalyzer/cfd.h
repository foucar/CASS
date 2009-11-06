#ifndef __SoftTDCCFD_h__
#define __SoftTDCCFD_h__

#include <iostream>
#include "waveform_analyzer.h"

namespace cass
{
    namespace REMI
    {
        //this is called in case it is a 8 Bit Instrument
        class CFD8Bit : public WaveformAnalyzer
        {
        public:
            CFD8Bit()    {std::cout << "using 8 bit Constant Fraction Discriminator"<<std::endl;}
            void analyze(Channel&, const double SampleInterval);
        };

        //this is called in case it is a 10 Bit Instrument
        class CFD16Bit : public WaveformAnalyzer
        {
        public:
            CFD16Bit()    {std::cout << "using 16 bit Constant Fraction Discriminator"<<std::endl;}
            void analyze(Channel&, const double SampleInterval);
        };

    }//end namespace remi
}//end namespace cass
#endif

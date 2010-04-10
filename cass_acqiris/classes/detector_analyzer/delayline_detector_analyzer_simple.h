//Copyright (C) 2009, 2010 Lutz Foucar

#ifndef __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_
#define __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_

#include "delayline_detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    //forward declarations//
    class DelaylineDetector;
    class AnodeLayer;

    /*! A simple reconstructor.
      class that will take an detector get the infos from it
      and then after creating the list of signals from the waveform uses the timesum
      to rekonstruct detectorhits from these signals.
      This is only done for two layers, even though a hex anode might also be used
      @todo after making sure that the waveform signal container will create the list of
            singals / peaks itselve, we no longer will need the info about the waveform
            analyzers in the constructor
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerSimple
        : public DelaylineDetectorAnalyzerBackend
    {
    public:
      /** constuctor outputs what we are*/
      DelaylineDetectorAnalyzerSimple(waveformanalyzers_t* waveformanalyzer)
          :DelaylineDetectorAnalyzerBackend(waveformanalyzer)
      {
        std::cout << "adding simple delayline detector analyzer"<<std::endl;
      }
      /** the function creating the detectorhit list*/
      virtual void operator()(DetectorBackend&,const Device&);
    };

  }//end namespace acqiris
}//end namespace cass

#endif

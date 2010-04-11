//Copyright (C)  2010 Lutz Foucar

#ifndef __TOF_ANALYZER_SIMPLE_H_
#define __TOF_ANALYZER_SIMPLE_H_

#include "detector_analyzer_backend.h"
#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! Simple Tof Analyzer

      will just take an event and feed it to the right
      waveform analyzer
      @note might not be needed anymore, once the list of peaks
            is created by the Signal itselve
      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT ToFAnalyzerSimple
      : public DetectorAnalyzerBackend
    {
    public:
      /** constructor
        @param[in] waveformanalyzer reference to the waveformanalyzer
                   container
      */
      ToFAnalyzerSimple(waveformanalyzers_t* waveformanalyzer)
        :DetectorAnalyzerBackend(waveformanalyzer)
      {}
      /** function that calles the right waveform analyzer*/
      virtual void operator()(DetectorBackend&,const Device&){};
    };
  }
}
#endif

// Copyright (C) 2010 Lutz Foucar

#ifndef _DETECTOR_ANALYZER_BACKEND_H_
#define _DETECTOR_ANALYZER_BACKEND_H_

#include <map>
#include "cass_acqiris.h"
#include "acqiris_device.h"

namespace cass
{
  namespace ACQIRIS
  {
    //forward declarations//
    class DetectorBackend;
    class WaveformAnalyzerBackend;
    /*! @brief Base class for all detector analyzers
      @todo check whether we need a base class, since it might be good
            if all detectors could have their own function
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT DetectorAnalyzerBackend
    {
    protected:
      /** @typedef waveformanalyzers_t make code more readable
        @todo might not be used anymore*/
      typedef std::map<WaveformAnalyzers, WaveformAnalyzerBackend*> waveformanalyzers_t;
    public:
      /** constructor needs to know what waveform analyzers are available*/
      DetectorAnalyzerBackend(waveformanalyzers_t* waveformanalyzer)
          :_waveformanalyzer(waveformanalyzer) {}
      /** virtual destructor*/
      virtual ~DetectorAnalyzerBackend() {}
      /** analyze the detector using the data from the device*/
      virtual void operator()(DetectorBackend&,const Device&)=0;
    protected:
      /** the map with all availabe detector analyzers*/
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

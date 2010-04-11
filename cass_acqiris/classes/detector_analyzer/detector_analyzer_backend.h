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
      @note we won't need a base class, since when we calc the values
            of all detectors lazyly they could have their own functions.
            Then one would only have to have a base class once there
            are several ways of analyzing the detector
      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT DetectorAnalyzerBackend
    {
    protected:
      /** typedef to make code more readable*/
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

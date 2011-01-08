// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class for all detector
 *                                   analyzers.
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_ANALYZER_BACKEND_H_
#define _DETECTOR_ANALYZER_BACKEND_H_


#include "cass_acqiris.h"
#include "delayline_detector.h"

namespace cass
{
  class CASSEvent;
  class CASSSettings;

  namespace ACQIRIS
  {
    //forward declarations//
    class DetectorBackend;

    /** Base class for all detector analyzers.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DetectorAnalyzerBackend
    {
    public:
      /** constructor needs to know what waveform analyzers are available*/
      DetectorAnalyzerBackend()  {}

      /** virtual destructor*/
      virtual ~DetectorAnalyzerBackend() {}

      /** combine all the signals from the detector to hits on the detector
       *
       * @return reference to the container containing the found hits
       * @param[out] hits the container where the found hits will go
       */
      virtual DelaylineDetector::hits_t& operator()(DelaylineDetector::hits_t &hits)=0;

      /** pure virtual function that will load the detector parameters from cass.ini*/
      virtual void loadSettings(CASSSettings&, DelaylineDetector&)=0;

      /** create an instance of the right analyzer type */
      static DetectorAnalyzerBackend* instance(const DetectorAnalyzerType);
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

// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class for all detector
 *                                   analyzers.
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_ANALYZER_BACKEND_H_
#define _DETECTOR_ANALYZER_BACKEND_H_

#include <map>
#include "cass_acqiris.h"
#include "acqiris_device.h"

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
      DetectorAnalyzerBackend()
      {}

      /** virtual destructor*/
      virtual ~DetectorAnalyzerBackend() {}

      /** analyze the detector using the data from the device
       *
       * @param detector the Detector whose signals should be analyzed.
       * @param device the device that recorded the signals of the Detector
       */
      virtual void operator()(DetectorBackend&,const Device&)=0;

      /** pure virtual function that will load the detector parameters from cass.ini*/
      virtual void loadSettings(CASSSettings&)=0;

      /** associate the event with this detector (get the data from this event) */
      virtual void associate(const CASSEvent&)=0;

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

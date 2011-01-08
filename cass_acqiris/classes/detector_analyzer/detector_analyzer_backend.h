// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class for all detector
 *                                   analyzers.
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_ANALYZER_BACKEND_H_
#define _DETECTOR_ANALYZER_BACKEND_H_

#include <memory>

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
     * @todo rename this class, since its just the base class for all (sorters?).
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DetectorAnalyzerBackend
    {
    public:
      /** virtual destructor */
      virtual ~DetectorAnalyzerBackend() {}

      /** retrieve detector hits from signals
       *
       * check the combination of signals to whether they belong together, since
       * the orign from the same detectorhit. Needs to be implemented by the
       * detectorhit finder (sorters?)
       *
       * @return reference to the container containing the found hits
       * @param[out] hits the container where the found hits will go
       */
      virtual detectorHits_t& operator()(detectorHits_t &hits)=0;

      /** load the settings of the analyzer
       *
       * load the settings from the .ini file. Needs to be implemented by the
       * detector that inherits from this.
       *
       * @param s reference to the CASSSettings object
       * @param d the detector object that we the analyzer belongs to
       */
      virtual void loadSettings(CASSSettings&, DelaylineDetector&)=0;

      /** create an instance of the right analyzer type
       *
       * this static member will create a instance of the requested type. If the
       * requested type is unknown an invalid_argument exception will be thrown.
       *
       * @return pointer to the instance of the requested type
       * @param type the requested analyzer type
       */
      static std::auto_ptr<DetectorAnalyzerBackend> instance(const DetectorAnalyzerType& type);
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

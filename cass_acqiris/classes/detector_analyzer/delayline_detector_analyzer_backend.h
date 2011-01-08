//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector_analyzer_backend.h file contains base class for all
 *                                             delayline detector analyzers.
 *
 * @author Lutz Foucar
 */

#ifndef __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_
#define __DELAYLINE_DETECTOR_ANALYZER_BACKEND_H_

#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** base class for delayline detector analysis.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerBackend
        : public DetectorAnalyzerBackend
    {
    public:
      /** constructor.
       *
       * @note does this need to know what kind of waveform analyzers are
       *       present? the list of Singals / Peak should be created by the
       *       class Signal itselve?
       */
      DelaylineDetectorAnalyzerBackend()
        :DetectorAnalyzerBackend()
      {}

      /** virtual destructor*/
      virtual ~DelaylineDetectorAnalyzerBackend() {}

      /** create the list of detector hits */
      virtual void operator()(DetectorBackend&,const Device&)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

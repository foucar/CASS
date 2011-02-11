//Copyright (C) 2011 Lutz Foucar

/**
 * @file delayline_non_sorting.h file contains the class that finds detectorhits
 *                               without sorting
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINENONSORTING_H_
#define _DELAYLINENONSORTING_H_

#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Simple detectorhit creator
     *
     * this class will just go through the anodewire signals and create
     * positions without any checks
     *
     * @author Lutz Foucar
     */
    class DelaylineNonSorting
      : public DetectorAnalyzerBackend
    {
    public:
      /** constructor */
      DelaylineNonSorting();

      /** retrieve detector hits from signals
       *
       * goes through the anodewire signals and calculates positions without
       * any checks.
       *
       * @return reference to the container containing the found hits
       * @param[out] hits the container where the found hits will go
       */
      detectorHits_t& operator()(detectorHits_t &hits);

      /** load the settings of the analyzer
       *
       * load where to find the signals that we need to sort
       *
       * @param s reference to the CASSSettings object
       * @param d the detector object that we the analyzer belongs to
       */
      void loadSettings(CASSSettings&, DelaylineDetector&);

    };
  }
}
#endif

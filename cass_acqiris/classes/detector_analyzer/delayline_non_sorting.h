//Copyright (C) 2011 Lutz Foucar

/**
 * @file delayline_non_sorting.h file contains the class that finds detectorhits
 *                               without sorting
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINENONSORTING_H_
#define _DELAYLINENONSORTING_H_

#include <utility>
#include <tr1/memory>

#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    class PositionCalculator;

    /** Simple detectorhit creator
     *
     * this class will just go through the anodewire signals and create
     * positions without any checks
     *
     * @cassttng .../NonSorting/{LayersToUse}\n
     *           Layers that should be used for sorting. Default is 0. Possible
     *           choises are:
     *           - 0: Layers X and Y (Quad Anode)
     *           - 1: Layers U and V (Hex Anode)
     *           - 2: Layers U and W (Hex Anode)
     *           - 3: Layers V and W (Hex Anode)
     * @cassttng .../NonSorting/{ScalefactorFirstLayer|ScalefactorSecondLayer}\n
     *           the scalefactors that convert ns to mm for the two layers.
     *           Default is 0.4 | 0.4
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

    private:
      /** the layer combination */
      std::pair<std::pair<SignalProducer *,SignalProducer *> ,
                std::pair<SignalProducer *,SignalProducer *> > _layerCombination;

      /** the calculator to calc the position for the correlated wireend signals */
      std::tr1::shared_ptr<PositionCalculator> _poscalc;

      /** the scalefactor for the two layers (convert ns -> mm) */
      std::pair<double,double> _sf;

    };
  }
}
#endif

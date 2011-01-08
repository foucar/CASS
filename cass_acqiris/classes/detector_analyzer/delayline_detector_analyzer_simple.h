//Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file delayline_detector_analyzer_simple.h file contains the declaration of
 *                                            classes that analyzses a delayline
 *                                            detector.
 *
 * @author Lutz Foucar
 */

#ifndef __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_
#define __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_

#include <memory>

#include "delayline_detector_analyzer_backend.h"
#include "delayline_detector.h"

namespace cass
{
  namespace ACQIRIS
  {
    class SignalProducer;
    class PositionCalculator;

    /** Simple sorter of hits from a Quadanode delayline detector.
     *
     * Do a simple sorting by checking the timesum for each MCP Signal that was
     * identified. This is done for only one pair of anode layers.
     *
     * @cassttng .../Simple/{Runtime}\n
     *           maximum time a signal will run over the complete delayline.
     *           Default is 150.
     * @cassttng .../Simple/{McpRadius}\n
     *           Radius of the MCP in ns. Default is 300
     * @cassttng .../Simple/{LayersToUse}\n
     *           Layers that should be used for sorting. Default is 0. Possible
     *           choises are:
     *           - 0: Layers X and Y (Quad Anode)
     *           - 1: Layers U and V (Hex Anode)
     *           - 2: Layers U and W (Hex Anode)
     *           - 3: Layers V and W (Hex Anode)
     * @cassttng .../Simple/{TimesumFirstLayerLow|TimesumFirstLayerHigh}\n
     *           the timesum condition range for the first layer.
     *           Default is 0 | 200
     * @cassttng .../Simple/{TimesumSecondLayerLow|TimesumSecondLayerHigh}\n
     *           the timesum condition range for the second layer.
     *           Default is 0 | 200
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerSimple
        : public DetectorAnalyzerBackend
    {
    public:
      /** constuctor */
      DelaylineDetectorAnalyzerSimple()
          :DetectorAnalyzerBackend(), _poscalc(0)
      {}

      /** the function creating the detectorhit list
       *
       * @return reference to the hit container
       * @param hits the hitcontainer
       */
      DelaylineDetector::hits_t& operator()(DelaylineDetector::hits_t &hits);

      /** function that will load the detector parameters from cass.ini
       *
       * retrieve all necessary information to be able to sort the signals of
       * the detector to detector hits. Also retrieve the signal producers of
       * the layers whos signals we should sort.
       *
       * @param s the CASSSetting object
       * @param d the detector object that we are belonging to
       */
      void loadSettings(CASSSettings &s, DelaylineDetector &d);

    private:
      /** the mcp */
      SignalProducer *_mcp;

      /** the layer combination */
      std::pair<std::pair<SignalProducer *,SignalProducer *> ,
                std::pair<SignalProducer *,SignalProducer *> > _layerCombination;

      /** timesum ranges of the layers */
      std::pair<std::pair<double, double>,
                std::pair<double, double> > _tsrange;

      /** timesums of the layers */
      std::pair<double,double> _ts;

      /** maximum runtime over the layers */
      double _runtime;

      /** maximum radius that det hits are allowed to be in, in ns */
      double _mcpRadius;

      /** the calculator to calc the position for the correlated wireend signals */
      std::auto_ptr<PositionCalculator> _poscalc;
    };


  }//end namespace acqiris
}//end namespace cass

#endif

//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector.h file contains the classes that describe a
 *                            delayline detector.
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINE_DETECTOR_H_
#define _DELAYLINE_DETECTOR_H_

#include <vector>
#include <algorithm>
#include <stdexcept>

#include "cass_acqiris.h"
#include "tof_detector.h"
#include "signal_producer.h"
#include "map.hpp"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    //forward declarations
    class DetectorAnalyzerBackend;

    /** A anode layer of the delayline detector.
     *
     * class containing the properties of a anode layer of the detector
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/\%Layername\%/
     *           {LowerTimesumConditionLimit|UpperTimesumConditionLimit}\n
     *           the timesum condition range for the layer.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%Layername\%/{Scalefactor}\n
     *           scalefactor to convert time => mm:
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT AnodeLayer
    {
    public:
      /** map of signals that form the wireends of the layer*/
      typedef std::map<char,SignalProducer> wireends_t;

      /** load values from cass.ini, should only be called by the detector*/
      void loadSettings(CASSSettings&);

      /** associate the event with this anodelayers signal producers */
      void associate(const CASSEvent&);

      /** retrieve the wireend */
      wireends_t        &wireend()      {return _wireend;}

    private:
      /*! the properties of the wireends, they are singals */
      wireends_t  _wireend;
    };








    /** A delayline detector.
     *
     * A delayline detector is a tof detector with the ability to also have
     * position information. It can be either a Hex or Quad delayline detector.
     * It contains all information that is needed in order to sort the signals
     * in the waveforms to detector hits.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/{Runtime}\n
     *           maximum time a signal will run over the complete delayline.
     * @cassttng AcqirisDetectors/\%detectorname\%/{Angle}\n
     *           Angle in degree by which on can rotate the picture around 0,0.
     *           Default is 0.
     * @cassttng AcqirisDetectors/\%detectorname\%/{McpRadius}\n
     *           Radius of the MCP in mm.
     * @cassttng AcqirisDetectors/\%detectorname\%/{AnalysisMethod}\n
     *           Method that is used to reconstruct the detector hits, choises are:
     *           - 0: Simple Analysis
     * @cassttng AcqirisDetectors/\%detectorname\%/{LayersToUse}\n
     *           Layers that should be used (when using the simple reconstruction method).
     *           - if HexAnode:
     *             - 0: Layers U and V
     *             - 1: Layers U and W
     *             - 2: Layers V and W
     *           - if QuadAnode (only one option available):
     *             - 0: Layers X and Y
     * @cassttng AcqirisDetectors/\%detectorname\%/{DeadTimeMcp}\n
     *           Dead time when detecting MCP Signals (used for future more advanced
     *           reconstruction methods)
     * @cassttng AcqirisDetectors/\%detectorname\%/{DeadTimeAnode}\n
     *           Dead time when detecting anode layer Signals (used for future more
     *           advanced reconstruction methods)
     * @cassttng AcqirisDetectors/\%detectorname\%/{WLayerOffset}\n
     *           The W-Layer offset with respect to layers U and V (used for future more
     *           advanced Hex-Detector reconstruction methods)
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetector : public TofDetector
    {
    public:
      /** a map of anodelayers */
      typedef std::map<char,AnodeLayer> anodelayers_t;
      typedef Map<std::string,double> hit_t;
      typedef std::vector<hit_t> hits_t;

    public:
      /** constructor.
       *
       * @param[in] type the delayline type is an enum either Hex or Quad
       * @param[in] name the name of this detector
       */
      DelaylineDetector(const std::string name)
        :TofDetector(name), _analyzer(0), _newEventAssociated(false)
      {}

    public:
      /** load the values from cass.ini*/
      virtual void loadSettings(CASSSettings&);

      /** associate the event with all of this detectors signal producers */
      void associate (const CASSEvent&);

      /** return the layers */
      anodelayers_t &layers() {return _anodelayers;}

      /** return the list of detector hits */
      hits_t &hits();

    private:
      /** delayline detector has anode wire layers */
      anodelayers_t _anodelayers;

      /** constainer for all reconstructed detector hits*/
      hits_t _hits;

      /** the analyzer that will sort the signal to hits */
      DetectorAnalyzerBackend * _analyzer;

      /** flag to show whether there is a new event associated whith this */
      bool _newEventAssociated;

    };

  }//end namespace remi
}//end namespace cass


#endif






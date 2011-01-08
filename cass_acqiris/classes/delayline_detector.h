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
     * class containing the wireends of an anode layer of the detector.
     *
     * This class has no user settable parameters. It will only open groups for
     * its different wireends. The groupname for the first wireend name is "One"
     * and for the second its "Two".
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/\%layername%/One\n
     *           groupname of the first wireend
     * @cassttng AcqirisDetectors/\%detectorname\%/\%layername%/Two\n
     *           groupname of the second wireend
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
     * It also contains detector hits. These are extracted by sorting the
     * signals of the singal producers. To do this each detector has its own
     * Analysis object. There are various ways to do this analysis, the user has
     * the option to choose which one he wants to use. Please refer to the
     * documentation of the different analyzers to find out their user settings.
     *
     * In addition to these parameters it will also opens the groups for the
     * different layers. The groupnames depend on the Delaylinetype. For a
     * Quad Anode it will be "XLayer" and "YLayer". For a Hex Anode it will be
     * "ULayer", "VLayer" and "WLayer". Please refer to AnodeLayer for the
     * user settable parameters of the anode layers.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/{AnalysisMethod}\n
     *           Method that is used to reconstruct the detector hits. Default
     *           is 0. Choises are:
     *           - 0: Simple Analysis (see DelaylineDetectorAnalyzerSimple)
     * @cassttng AcqirisDetectors/\%detectorname\%/{DelaylineType}\n
     *           What kind of Delaylinedetector are we. Default is 0
     *           - 0: Quad Anode
     *           - 1: Hex Anode
     * @cassttng AcqirisDetectors/\%detectorname\%/XLayer\n
     *           groupname of the X Layer, when DelaylineType is Quad
     * @cassttng AcqirisDetectors/\%detectorname\%/YLayer\n
     *           groupname of the Y Layer, when DelaylineType is Quad
     * @cassttng AcqirisDetectors/\%detectorname\%/ULayer\n
     *           groupname of the U Layer, when DelaylineType is Hex
     * @cassttng AcqirisDetectors/\%detectorname\%/VLayer\n
     *           groupname of the V Layer, when DelaylineType is Hex
     * @cassttng AcqirisDetectors/\%detectorname\%/WLayer\n
     *           groupname of the W Layer, when DelaylineType is Hex
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

      /** destroys the detector analyzer */
      ~DelaylineDetector();


    public:
      /** load the values from the .ini file
       *
       * this function will load the settings of this detector, which are
       * described in the class description. Next to them it will load the
       * settings of its SignalProducers (mcp and anodelayers). Please refer to
       * SignalProducer::loadSettings() for further information.\n
       * Then it will create the requested analyzer by calling
       * DetectorAnalyzerBackend::instance() and load the settings for the
       * analyzer. Please refer to the analyzers loadSettings() member for
       * further information.
       *
       * @param s the CASSSettings object we retrieve the values from
       */
      virtual void loadSettings(CASSSettings &s);

      /** associate the event with this detector
       *
       * when this is called, it means that a data from a new event will be
       * available. Therefore the _newEventAssociatad is set to true and the
       * _hits container is cleared. Then the Signalproduers of this detector
       * (the mcp and all anodlayers) will be associated with this event. Please
       * refer to SignalProducer::associate() for further information.
       *
       * @param evt The event to associate with this detector
       */
      void associate (const CASSEvent& evt);

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






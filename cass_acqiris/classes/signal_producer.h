//Copyright (C) 2010 Lutz Foucar

/**
 * @file waveform_signal.h file contains the classes that describe how to
 *                         analyze the waveform and stores the result.
 *
 * @author Lutz Foucar
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include "cass_acqiris.h"
#include "peak.h"
#include "map.hpp"

namespace cass
{
  //forward declaration
  class CASSSettings;
  class CASSEvent;

  namespace ACQIRIS
  {
    class SignalExtractor;

    /** A Signal Producer.
     *
     * This class describes all signal producing elements of a detector. It
     * contains an extractor for the produced signals from the event and a list
     * of the signals it produced.
     *
     * User settable parameters via CASS.ini
     * - general access to these parameters depends on the detector type:
     *   - In Delaylinedetectors its for
     *      - MCP: AcqirisDetectors/%detectorname%/MCP
     *      - Layers Wireends: AcqirisDetectors/%detectorname%/%Layername%/%Wireendname%
     *   - In TofDetectors: AcqirisDetectors/%detectorname%/MCP
     *
     * Then the specific settings for these objects are:
     * @cassttng .../{SignalExtractionMethod}\n
     *           the method type that will be used to extract the signals from
     *           the recorded data. There are the following options :
     *           - 0:com 8 bit waveform
     *           - 1:com 16 bit waveform
     *           - 2:cfd 8 bit waveform
     *           - 3:cfd 16 bit waveform
     *           depending on the type of analyzer one has to set up the analyzer
     *           parameters
     * @cassttng .../{LowerGoodTimeRangeLimit|UpperGoodTimeRangeLimit}\n
     *           time range of the channel that "good" signals will appear. This
     *           is used by delayline detectors for displaying the first good
     *           hits and the timesum.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT SignalProducer
    {
    public:
      typedef Map<std::string,double> signal_t;
      typedef std::vector<signal_t> signals_t;

    public:
      /** default constructor.
       *
       * intializing the variables describing the extraction with nonsense,
       * since they have to be loaded by loadSettings from cass.ini
       */
      SignalProducer()
        :_signalextractor(0),_newEventAssociated(false)
      {}

    public:
      /** loads the parameters.
       *
       * load the parameters from cass.ini, should only be called by class
       * containing this class
       */
      void loadSettings(CASSSettings &p);

      /** assciate the event with this signalproducer
       *
       * resets the _newEventAssociated flag to true, clears the _signals vector
       * and associates the event with the signalextractor.
       *
       * @param evt the event that we need to associate with the signalextractor
       */
      void associate(const CASSEvent& evt);

    public:
      /** return the time of the first peak in the "good" time range*/
      double firstGood(const std::pair<double,double>& range);

      /** return the signals
       *
       * When a new event was associated with this prodcuer, then it will first
       * extract all signals from the event data otherwise it will just return
       * the signals
       *
       * @note the output of a signal producer are the singals. Unfortunately
       *       if we call this function signals it will not compile anymore.
       */
      signals_t& output();

    private:
      /** time of the first peak in the "good" range*/
      double _goodHit;

      /** the extractor of the produced signals */
      SignalExtractor * _signalextractor;

      /** the signals produces by this producer */
      signals_t _signals;

      /** flag to show whether there is a new event associated whith this signal producer */
      bool _newEventAssociated;
    };
  }//end namespace acqiris
}//end namespace cass


#endif

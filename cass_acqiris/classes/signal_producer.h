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
#include <tr1/memory>
#include <map>
#include <vector>
#include <string>

#include "cass_acqiris.h"
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
     *           the recorded data. To see what options need to be set for the
     *           specific signalextraction methods description.
     *           There are the following options :
     *           - 0:com 8 bit waveform (see CoM8Bit)
     *           - 1:com 16 bit waveform (see CoM8Bit)
     *           - 2:cfd 8 bit waveform (see CFD8Bit)
     *           - 3:cfd 16 bit waveform (see CFD8Bit)
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT SignalProducer
    {
    public:
      typedef Map<std::string,double> signal_t;
      typedef std::vector<signal_t> signals_t;

    public:
      /** default constructor */
      SignalProducer()
        :_goodHit(0),
         _newEventAssociated(false),
         _goodHitExtracted(false)
      {}

    public:
      /** loads the settings.
       *
       * will load the the requested SignalExtractor by calling
       * SignalExtractor::instance(). And then loads its settings. Please refer
       * the the chosen signal extractors loadSettings memeber for further
       * information.\n
       * See class describtion for the type of signalextractor that can be
       * chosen.
       *
       * @param s the CASSSettings object we retrieve the data from
       */
      void loadSettings(CASSSettings &s);

      /** assciate the event with this signalproducer
       *
       * resets the _newEventAssociated flag to true, clears the _signals vector
       * and associates the event with the signalextractor. See the signal
       * extractors associate() member function for further information.
       *
       * @param evt the event that we need to associate with the signalextractor
       */
      void associate(const CASSEvent& evt);

    public:
      /** returns the time of the first peak in the time range
       *
       * when the _newEventAssociated flag is true it will look for the first
       * signal whos time is in the requested time range. If there is no signal
       * in the requested timerange the value is set to 0.
       *
       * @return time of the first singal in the requested timerange
       * @param range the timerange to search for the signal
       */
      double firstGood(const std::pair<double,double>& range);

      /** return the signals
       *
       * When a new event was associated with this prodcuer, then it will first
       * extract all signals from the event data otherwise it will just return
       * the signals. It will extract the events from the data with the help of
       * the _singalExtractor object that this class owns. This is done by
       * calling the singalextractos operator().
       *
       * @note the output of a signal producer are the singals. Unfortunately
       *       if we call this function signals it will not compile anymore.
       *
       * @return reference to the singals of this singalproducer
       */
      signals_t& output();

    private:
      /** time of the first peak in the "good" range*/
      double _goodHit;

      /** the extractor of the produced signals */
      std::tr1::shared_ptr<SignalExtractor> _signalextractor;

      /** the signals produces by this producer */
      signals_t _signals;

      /** flag to show whether there is a new event associated whith this signal producer */
      bool _newEventAssociated;

      /** flag to show whether the first good hit has been extracted */
      bool _goodHitExtracted;
    };
  }//end namespace acqiris
}//end namespace cass


#endif

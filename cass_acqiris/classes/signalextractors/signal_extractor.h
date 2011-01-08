//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_extractor.h file file contains base class for all classes that
 *                          extract signals from the recorded data
 *
 * @author Lutz Foucar
 */

#ifndef _SIGNAL_EXTRACTOR_H_
#define _SIGNAL_EXTRACTOR_H_

#include "cass_acqiris.h"
#include "signal_producer.h"

namespace cass
{
  class CASSEvent;
  class CASSSettings;

  namespace ACQIRIS
  {
    /** Base class for classes that extract Signals from recorded data
     *
     * waveform analyzers should take a channel, analyze its wavefrom
     * and put the result of the analysis into the results base class
     *
     * @todo rename this to reflect its purpose better (it not only is used for
     *       waveforms, but also for extracting the right signals from a tdc
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT SignalExtractor
    {
    public:
      /** virtual destructor*/
      virtual ~SignalExtractor(){}

      /** pure virtual function stub for all analyzers extract signals form the
       * CASSEvent.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      virtual SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig) = 0;

      /** associate the event with this analyzer */
      virtual void associate(const CASSEvent& evt)=0;

      /** load the settings of the extractor */
      virtual void loadSettings(CASSSettings&)=0;

      /** creates an instance of the requested analyzer type */
      static SignalExtractor* instance(SignalExtractorType);
    };
  }//end namespace acqiris
}//end namespace cass


#endif

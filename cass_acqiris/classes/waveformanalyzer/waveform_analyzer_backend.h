//Copyright (C) 2010 Lutz Foucar

/**
 * @file waveform_analyzer_backend.h file contains base class for all wavefrom
 *                                   analyzers
 *
 * @author Lutz Foucar
 */

#ifndef _WAVEFORM_ANALYZER_BACKEND_H_
#define _WAVEFORM_ANALYZER_BACKEND_H_

#include "cass_acqiris.h"
#include "signal_producer.h"

namespace cass
{
  class CASSEvent;

  namespace ACQIRIS
  {
    /** Base class for Wavefrom analyzers
     *
     * waveform analyzers should take a channel, analyze its wavefrom
     * and put the result of the analysis into the results base class
     *
     * @todo rename this to reflect its purpose better (it not only is used for
     *       waveforms, but also for extracting the right signals from a tdc
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT WaveformAnalyzerBackend
    {
    public:
      /** virtual destructor*/
      virtual ~WaveformAnalyzerBackend(){}

      /** pure virtual function stub for all analyzers extract signals form the
       * CASSEvent.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      virtual SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig) = 0;

      /** associate the event with this analyzer */
      virtual void associate(const CASSEvent& evt)=0;
    };
  }//end namespace acqiris
}//end namespace cass
#endif

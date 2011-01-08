//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_extractor.h file file contains base class for all classes that
 *                          extract signals from the recorded data
 *
 * @author Lutz Foucar
 */

#ifndef _SIGNAL_EXTRACTOR_H_
#define _SIGNAL_EXTRACTOR_H_

#include <tr1/memory>

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
     * All classes that want to extract signals from the data, should inherit
     * from this class. This classes operator will be called by the signal
     * producers, that want to have theier singals extracted from the data.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT SignalExtractor
    {
    public:
      /** virtual destructor */
      virtual ~SignalExtractor(){}

      /** retrieve signals from data
       *
       * extract signals form the CASSEvent. Needs to be implemented by the
       * classes that inerhit from this.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      virtual SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig) = 0;

      /** associate the event with this analyzer
       *
       * retrieve all necessary information from the event to be able to later
       * extract the signals from the data. Needs to be implemented by the class
       * that implements the extractor method.
       *
       * @param evt the event the signals are extracted from
       */
      virtual void associate(const CASSEvent& evt)=0;

      /** load the settings of the extractor
       *
       * load the settings form the .ini file. Needs to be implementd by the
       * class that implements the signal extractor.
       *
       * @param s the CASSSettings object to retrieve the information from.
       */
      virtual void loadSettings(CASSSettings &s)=0;

      /** creates an instance of the requested extractor type
       *
       * this static member will create an instance of the requested type, which
       * is a class that inherits from this. If the requested type is unknown,
       * an invalid_argument exception will be thrown.
       *
       * @return shared_ptr to the instance of the requested type
       * @param type The type of signal extractor that the user requests
       */
      static std::tr1::shared_ptr<SignalExtractor> instance(SignalExtractorType type);
    };
  }//end namespace acqiris
}//end namespace cass


#endif

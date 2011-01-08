//Copyright (C) 2010 Lutz Foucar

/**
 * @file tof_detector.h file contains the declaration of the class that
 *                      describes a Time Of Flight Detector.
 *
 * @author Lutz Foucar
 */

#ifndef _TOF_DETECTOR_H_
#define _TOF_DETECTOR_H_

#include "detector_backend.h"
#include "signal_producer.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** A Time of Flight Detector.
     *
     * for user settable settings see Signal
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT TofDetector : public DetectorBackend
    {
    public:
      /** constructor.
       *
       * @param[in] name the name of the detector
       */
      TofDetector(const std::string name)
        :DetectorBackend(name)
      {}

      /** virtual destructor*/
      virtual ~TofDetector() {}

      /** associate the event with this detector (get the data from this event) */
      virtual void associate (const CASSEvent&);

      /** load the values from cass.ini */
      virtual void loadSettings(CASSSettings&);

      /** getter for the signal*/
      const SignalProducer &mcp()const {return _mcp;}

      /** setter for the singal*/
      SignalProducer &mcp() {return _mcp;}

    protected:
      /** the properties of the mcp of the tofdetector*/
      SignalProducer _mcp;
    };
  }
}


#endif

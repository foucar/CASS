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
     * A Time of Flight Detector has only one component that produces signals
     * that are recorded, the mcp. The SignalProducer (_mcp) does know how to
     * extract the singals from the recoreded data. The function can be set up
     * in the signal producers CASSSettings settings. Please refer to
     * SingalProducer class describtion for further details.
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

      /** virtual destructor */
      virtual ~TofDetector() {}

      /** associate the event with this detector
       *
       * Since all the data in just enclosed in the mcp, this function will just
       * call the mcp associate member. Please refer to
       * SignalProducer::associate() for further details.
       *
       * @param evt the event to whos data we associated to this detector
       */
      virtual void associate (const CASSEvent &evt);

      /** load the values from .ini file
       *
       * will just open the group named MCP and then call the member
       * SingalProducer::loadSettings() of the _mcp.
       *
       * @param s the CASSSettings object to read the information from
       */
      virtual void loadSettings(CASSSettings &s);

      /** retrieve the mcp */
      SignalProducer &mcp() {return _mcp;}

      /** retrieve the detector type */
      DetectorType type() {return ToF;}

    protected:
      /** the mcp of the detector */
      SignalProducer _mcp;
    };
  }
}


#endif

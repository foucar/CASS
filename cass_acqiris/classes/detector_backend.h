// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_backend.h contains the base class declaration for all detectors
 *                          that are attached to an acqiris device.
 *
 * @todo create another acqiris detector, that will just measure the voltage
 *       on a given channel
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include <iostream>
#include "cass_acqiris.h"
#include "cass_settings.h"

namespace cass
{
  namespace ACQIRIS
  {
    class DetectorAnalyzerBackend;
    class Device;

    /** Base class for all Detectors attached to an Acqiris Instrument.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DetectorBackend
    {
    public:
      /** constructor.
       *
       * @param[in] name the name of the detector
       */
      DetectorBackend(const std::string name)
          :_name(name),_analyzer(0)
      {}

      /** virtual destructor*/
      virtual ~DetectorBackend() {}

      /** pure virtual function that will load the detector parameters from cass.ini*/
      virtual void loadSettings(CASSSettings*)=0;

      /** operator that will calculate everything from the event for this detector */
      virtual void operator() (const Device& device)=0;

      /** pure virtual assignment operator.*/
      virtual DetectorBackend& operator= (const DetectorBackend&)=0;

      /** the detector name*/
      const std::string name()const {return _name;}

      /** clear the detector data */
      virtual void clear()=0;

    protected:
      /** the name of the detector. used for casssettings group*/
      std::string _name;

      /** pointer to the analyzer that is used for analyzing this detector */
      DetectorAnalyzerBackend *_analyzer;

    private:
      /** default constructor should not be called therefore its privat*/
      DetectorBackend():_name("unamed") {}
    };
  }
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

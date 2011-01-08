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
  class CASSEvent;

  namespace ACQIRIS
  {
    class DetectorAnalyzerBackend;

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
          :_name(name)
      {}

      /** virtual destructor*/
      virtual ~DetectorBackend() {}

      /** pure virtual function that will load the detector parameters from cass.ini*/
      virtual void loadSettings(CASSSettings&)=0;

      /** associate the event with this detector (get the data from this event) */
      virtual void associate(const CASSEvent&)=0;

      /** the detector name*/
      const std::string name()const {return _name;}

      /** create an instance of the requested dectortype
       *
       * @return an instance of the the requested detector type.
       * @param dettype type that the detector should have
       * @param detname the name of the detector in the .ini file
       */
      static DetectorBackend* instance(const DetectorType &dettype, const std::string &detname);

    protected:
      /** the name of the detector. used for casssettings group*/
      std::string _name;

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

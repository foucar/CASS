// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_backend.h contains the base class declaration for all detectors
 *                          that are attached to an acqiris device.
 *
 * @todo create another acqiris detector, that will just measure the voltage
 *       on a given channel
 * @todo use shared pointers
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include <iostream>
#include <tr1/memory>

#include "cass_acqiris.h"

namespace cass
{
class CASSEvent;
class CASSSettings;

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
  /** a shared pointer of this type */
  typedef std::tr1::shared_ptr<DetectorBackend> shared_pointer;

protected:
  /** constructor.
   *
   * @param[in] name the name of the detector
   */
  DetectorBackend(const std::string name)
    :_name(name)
  {}

public:
  /** virtual destructor*/
  virtual ~DetectorBackend() {}

  /** load the settings of the detector
   *
   * load the settings from the .ini file. Needs to be implemented by the
   * detector that inherits from this.
   *
   * @param s reference to the CASSSettings object
   */
  virtual void loadSettings(CASSSettings &s)=0;

  /** associate the event with this detector
   *
   * retrieve all necessary information for this detector from the event.
   * Needs to be implemented by the detector inheriting from this.
   *
   * @param evt The event to take the data from
   */
  virtual void associate(const CASSEvent& evt)=0;

  /** return the detector name*/
  const std::string name()const {return _name;}

  /** create an instance of the requested dectortype
   *
   * if the requested detector type is not known an exception will be thrown
   *
   * @return an instance of the the requested detector type.
   * @param dettype type that the detector should have
   * @param detname the name of the detector in the .ini file
   */
  static shared_pointer instance(const DetectorType &dettype, const std::string &detname);

  /** retrieve what kind of detector this is */
  virtual DetectorType type()=0;

protected:
  /** the name of the detector. used for casssettings group*/
  std::string _name;

private:
  /** default constructor should not be called therefore its privat*/
  DetectorBackend():_name("unamed") {}
};
}//end namespace acqiris
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

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

#include "acqiris_analysis_definitions.hpp"

namespace cass
{
class CASSEvent;
class CASSSettings;

namespace ACQIRIS
{
class DetectorAnalyzerBackend;

/** Base class for all Detectors attached to an Acqiris Instrument.
 *
 * @todo this class can have the conatiner for all signal producers. It can
 *       also have the container for all particles and detectorhits. In case
 *       it is a tof detector it just won't have the px and py components and
 *       related values. This sig prod will just be call either "u1" .. so the
 *       map is of string,sigprod instead of char,sigprod. When changing this
 *       also change the directory structure by flatening it out so that one
 *       can call make -j. (Or create a .pro file for each subfolder)
 *
 * @author Lutz Foucar
 */
class DetectorBackend
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

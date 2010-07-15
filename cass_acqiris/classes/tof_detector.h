//Copyright (C) 2010 Lutz Foucar

/**
 * @file tof_detector.h file contains the classes that describe a Time Of
 *                      Flight Detector.
 *
 * @author Lutz Foucar
 */

#ifndef _TOF_DETECTOR_H_
#define _TOF_DETECTOR_H_

#include "cass_acqiris.h"
#include "detector_backend.h"
#include "waveform_signal.h"
#include "cass_settings.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** A Time of Flight Detector.
     *
     * for user settable settings @see Signal
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
      {_analyzerType = ToFSimple;}

      /** virtual destructor*/
      virtual ~TofDetector() {}

      /** overwrite the DetectorBackend assignment operator.
       *
       * this is needed to provide easy copying of this members in
       * the Helper function of Acqiris. @see cass::HelperAcqirisDetectors::validate
       */
      virtual DetectorBackend& operator= (const DetectorBackend& rhs);

      /** load the values from cass.ini */
      virtual void loadSettings(CASSSettings *p);

      /** save values to cass.ini */
      virtual void saveParameters(CASSSettings *){}

      /** getter for the signal*/
      const Signal        &mcp()const {return _mcp;}

      /** setter for the singal*/
      Signal              &mcp() {return _mcp;}

    protected:
      /** the properties of the mcp of the tofdetector*/
      Signal _mcp;
    };
  }
}

inline
void cass::ACQIRIS::TofDetector::loadSettings(CASSSettings *p)
{
  p->beginGroup(_name.c_str());
  _mcp.loadSettings(p,"Signal");
  p->endGroup();
}

inline
cass::ACQIRIS::DetectorBackend& cass::ACQIRIS::TofDetector::operator= (const cass::ACQIRIS::DetectorBackend& rhs)
{
  //if we are not self assigning//
  if (&rhs != this)
  {
    //copy the signal from the right hand side
    //to the signal that belongs to this//
    _mcp          = dynamic_cast<const TofDetector&>(rhs)._mcp;
    //backend's stuff//
    _analyzerType = dynamic_cast<const TofDetector&>(rhs)._analyzerType;
    _name         = dynamic_cast<const TofDetector&>(rhs)._name;

  }
  //return a reference to this//
  return *this;
}

#endif

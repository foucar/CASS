//Copyright (C) 2010 Lutz Foucar

#ifndef _TOF_DETECTOR_H_
#define _TOF_DETECTOR_H_

#include "cass_acqiris.h"
#include "detector_backend.h"
#include "waveform_signal.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! A Time of Flight Detector

      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT TofDetector : public DetectorBackend
    {
    public:
      /** constructor
        @param[in] name the name of the detector
      */
      TofDetector(const std::string name)
        :DetectorBackend(name)
      {_analyzerType = ToFSimple;}
      /** load the values from cass.ini */
      virtual void loadParameters(QSettings *p);
      /** save values to cass.ini */
      virtual void saveParameters(QSettings *){};
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
void cass::ACQIRIS::TofDetector::loadParameters(QSettings *p)
{
  p->beginGroup(_name.c_str());
  _mcp.loadParameters(p,"Signal");
  p->endGroup();
}

#endif

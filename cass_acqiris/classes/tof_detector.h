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
      /** constructor*/
      TofDetector()
        :DetectorBackend(ToF)
      {_analyzerType = ToFSimple;}
      /** load the values from cass.ini */
      virtual void loadParameters(QSettings *p){};
      /** save values to cass.ini */
      virtual void saveParameters(QSettings *p){};
    private:
      /** the actual tof peaks*/
      Signal _tof;
    };
  }
}

#endif

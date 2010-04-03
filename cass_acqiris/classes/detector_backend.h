// Copyright (C) 2010 Lutz Foucar

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include <iostream>
#include <QtCore/QSettings>
#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! @brief Base class for all Detectors attached to an Acqiris Instrument
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT DetectorBackend
    {
    public:
      /** while creating the backend we need to know what kind of detector we are*/
      DetectorBackend(DetectorType type)
          :_type(type)           {}
      /** virtual desctructor*/
      virtual ~DetectorBackend() {}
      /** pure virtual function that will load the detector parameters from cass.ini*/
      virtual void loadParameters(QSettings*)=0;
      /** pure virtual function that will save the detector parameters to cass.ini
        @todo check whether we need to save settings at all, since the program should
              not change any parameters*/
      virtual void saveParameters(QSettings*)=0;
      /** the type of analysis used to analyze this detector
        @todo check whether we need this, since the detector should
              calculate its properties it selve*/
      DetectorAnalyzers    analyzerType()const    {return _analyzerType;}
      DetectorAnalyzers   &analyzerType()         {return _analyzerType;}
      /** @returns  the detector type*/
      DetectorType         type()const            {return _type;}
    protected:
      /** which analyzer should be used*/
      DetectorAnalyzers _analyzerType;
      /** what typ is this detector*/
      DetectorType _type;
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

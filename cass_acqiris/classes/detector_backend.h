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

      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT DetectorBackend
    {
    public:
      /** constructor
        @param[in] name the name of the detector
      */
      DetectorBackend(const std::string name)
          :_name(name)
      {}
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
      /** the detector name*/
      const std::string name()const {return _name;}
    protected:
      /** which analyzer should be used*/
      DetectorAnalyzers _analyzerType;
      /** the name of the detector. used for qsettings group*/
      std::string _name;
    private:
      /** default constructor should not be called therefore its privat*/
      DetectorBackend()
          :_name("unamed")
      {}
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

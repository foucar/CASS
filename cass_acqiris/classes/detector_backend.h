// Copyright (C) 2010 lmf

#ifndef _DETECTOR_BACKEND_H_
#define _DETECTOR_BACKEND_H_

#include <QtCore/QSettings>
#include "cass_acqiris.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT DetectorBackend
    {
    public:
      DetectorBackend(DetectorType type)
          :_type(type)           {}
      virtual ~DetectorBackend() {}
      virtual void loadParameter(const QSettings*)=0;
      virtual void saveParameter(QSettings*)=0;
      DetectorAnalyzers    analyzerType()const    {return _analyzerType;}
      DetectorAnalyzers   &analyzerType()         {return _analyzerType;}
      DetectorTyp          type()const            {return _type;}
    protected:
      DetectorAnalyzers    _analyzerType; //which analyzer should be used
      DetectorType         _type;         //what typ is this detector
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

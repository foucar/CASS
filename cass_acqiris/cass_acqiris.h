//jk,lmf

#ifndef _ACQIRIS_GLOBAL_H_
#define _ACQIRIS_GLOBAL_H_


#include <QtCore/qglobal.h>

#if defined(CASS_ACQIRIS_LIBRARY)
#  define CASS_ACQIRISSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_ACQIRISSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass
{
  namespace ACQIRIS
  {
    enum Polarity {Bad,Positive,Negative};
    enum LayersToUse {UV,UW,VW};
    enum DetectorAnalyzers {DelaylineSimple};
    enum DetectorType {Delayline, ToF};
    enum DelaylineType {Quad,Hex};
    enum WaveformAnalyzers {com8,com16,cfd8,cfd16};
    enum LayerTypes{U,V,W};
  }
}

#endif // CASS_ACQIRIS_GLOBAL_H

#ifndef _ACQIRIS_GLOBAL_H_
#define _ACQIRIS_GLOBAL_H_

//lmf

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
    enum DetectorAnalyzers {DelaylineDetectorSimple};
    enum DetectorTypes {DelaylineDetector, ToFDetector};
    enum WaveformAnalyzers {CoM8Bit,CoM16Bit,CFD8Bit,CFD16Bit};
    enum LayerTypes{U,V,W};
  }
}

#endif // CASS_ACQIRIS_GLOBAL_H

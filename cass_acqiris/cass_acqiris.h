//Copyright (C) 2009 Jochen Kuepper
//Copyright (C) 2009,2010 lmf

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
    /** the Polarity of a Signal in the waveform (Peak)*/
    enum Polarity {Bad,Positive,Negative};
    /** the layercombination choice for evaluating the detector hits
    this is used by the simple delayline detektor hit sorter*/
    enum LayersToUse {UV,UW,VW};
    /** the types of detectors that are available */
    enum DetectorType {Delayline, ToF};
    /** the types of delayline detectors that are available */
    enum DelaylineType {Quad,Hex};
    /** the waveformanalyzers that are available */
    enum WaveformAnalyzers {com8,com16,cfd8,cfd16};
    /** all available instruments at the site*/
    enum Instruments{Camp1=8,Camp2=4,Camp3=5};
    /** all available detectors*/
    enum Detectors{HexDetector, QuadDetector, VMIMcp, IntensityMonitor, Photodiode};
    /** the available detector analyzers*/
    enum DetectorAnalyzers {DelaylineSimple};
  }
}

#endif // CASS_ACQIRIS_GLOBAL_H

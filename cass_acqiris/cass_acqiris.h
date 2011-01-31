//Copyright (C) 2009 Jochen Kuepper
//Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file cass_acqiris.h file contains the global definitions for the acqiris
 *                      library
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRIS_GLOBAL_H_
#define _ACQIRIS_GLOBAL_H_

#include <vector>
#include <stdint.h>
#include <string>

#include <QtCore/qglobal.h>

#include "pdsdata/xtc/DetInfo.hh"

#include "map.hpp"

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
    /** the types of detectors that are available */
    enum DetectorType {Delayline, ToF};
    /** the available detector analyzers*/
    enum DetectorAnalyzerType{DelaylineSimple};
    /** the types of delayline detectors that are available */
    enum DelaylineType {Quad,Hex};
    /** the waveformanalyzers that are available */
    enum SignalExtractorType {com8,com16,cfd8,cfd16,tdcextractor};
    /** all available instruments at the site*/
    enum Instruments{
      Camp1 = Pds::DetInfo::Camp,
      Camp2 = Pds::DetInfo::AmoITof,
      Camp3 = Pds::DetInfo::AmoMbes,
      Camp4 = Pds::DetInfo::AmoGasdet
    };
   //@{
    /** typdef for better readable code */
    typedef std::vector<int16_t> waveform_t;
    typedef Map<std::string,double> particleHit_t;
    typedef std::vector<particleHit_t> particleHits_t;
    typedef Map<std::string,double> detectorHit_t;
    typedef std::vector<detectorHit_t> detectorHits_t;
    //@}
  }

  namespace ACQIRISTDC
  {
    /** all available tdc instruments at the site*/
    enum TDCInstruments{
      SXRTdc = Pds::DetInfo::SxrEndstation
    };
  }

}

#endif // CASS_ACQIRIS_GLOBAL_H

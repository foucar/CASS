//Copyright (C) 2009 Jochen Kuepper
//Copyright (C) 2009, 2010, 2011, 2013 Lutz Foucar

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
enum DetectorAnalyzerType{DelaylineSimple,AchimsRoutine,AchimsCalibrator,NonSorting};
/** the types of delayline detectors that are available */
enum DelaylineType {Quad,Hex};
/** the waveformanalyzers that are available */
enum SignalExtractorType {com8,com16,cfd8,cfd16,tdcextractor};
/** all available instruments at the site*/
enum Instruments{
  Camp1  = Pds::DetInfo::Camp,
  Camp2  = Pds::DetInfo::AmoITof,
  Camp3  = Pds::DetInfo::AmoMbes,
  Camp4  = Pds::DetInfo::AmoGasdet,
  XPP    = Pds::DetInfo::XppLas,
  Standalone = 1
};
/** types of pariticle hits */
enum particleHits
{
  px              =  0,
  py              =  1,
  pz              =  2,
  x_mm            =  4,
  y_mm            =  5,
  tof_ns          =  6,
  xCor_mm         =  7,
  yCor_mm         =  8,
  xCorScal_mm     =  9,
  yCorScal_mm     = 10,
  xCorScalRot_mm  = 11,
  yCorScalRot_mm  = 12,
  tofCor_ns       = 13,
  roh             = 14,
  theta           = 15,
  phi             = 16,
  e_au            = 17,
  e_eV            = 18,
  NbrParticleHitDefinitions
};
/** types of detector hits */
enum detectorHits
{
  x       =  0,
  y       =  1,
  t       =  2,
  method  =  3,
  NbrDetectorHitDefinitions
};
/** types of signals */
enum SignalProperties
{
  time        =  0,
  cfd         =  1,
  polarity    =  2,
  isUsed      =  3,
  com         =  4,
  fwhm        =  5,
  height      =  6,
  maxpos      =  7,
  width       =  8,
  startpos    =  9,
  stoppos     = 10,
  maximum     = 11,
  integral    = 12,
  NbrSignalDefinitions
};

/** fuzzy compare two floating point variables
 *
 * @tparam the type that one want to compare
 * @param first the first value for the equal comparison
 * @param second the second value for the equal comparison
 */
template <typename T>
bool fuzzycompare(const T& first, const T& second)
{
  return (abs(first-second) < sqrt(std::numeric_limits<T>::epsilon()));
}

/** fuzzy compare a floating point number to 0
 *
 * @tparam the type that one want to compare
 * @param val the value for the comparison
 */
template <typename T>
bool fuzzyIsNull(const T& val)
{
  return (val < sqrt(std::numeric_limits<T>::epsilon()));
}

//@{
/** typdef for better readable code */
typedef std::vector<int16_t> waveform_t;
typedef std::vector<double> particleHit_t;
typedef std::vector<particleHit_t> particleHits_t;
typedef std::vector<double> detectorHit_t;
typedef std::vector<detectorHit_t> detectorHits_t;
//@}
}
}

#endif // CASS_ACQIRIS_GLOBAL_H

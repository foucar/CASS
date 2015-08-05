//Copyright (C) 2015 Lutz Foucar

/**
 * @file acqiris_analysis_definitions.hpp contains the global definitions
 *                                        for acqiris analysis
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRIS_ANALYSIS_DEFINITIONS_HPP_
#define _ACQIRIS_ANALYSIS_DEFINITIONS_HPP_

#include <vector>
#include <stdint.h>
#include <string>
#include <limits>

namespace cass
{
namespace ACQIRIS
{
/** define a particle hit */
typedef std::vector<double> particleHit_t;

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
/** define container for all particle hits */
typedef std::vector<particleHit_t> particleHits_t;

/** define a detector hit */
typedef std::vector<double> detectorHit_t;

/** types of detector hits */
enum detectorHits
{
  x       =  0,
  y       =  1,
  t       =  2,
  method  =  3,
  NbrDetectorHitDefinitions
};

/** define container for all detector hits */
typedef std::vector<detectorHit_t> detectorHits_t;

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

} //end namespace ACQIRIS
} //end namespace cass
#endif


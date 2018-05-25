// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.cpp contains base class implementation
 *                                   for all detector analyzers.
 *
 * @author Lutz Foucar
 */

#include <sstream>

#include "detector_analyzer_backend.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_non_sorting.h"
#include "cass.h"
#ifdef ACHIMSRESORTER
#include "achimsorter_hex.h"
#endif

using namespace cass::ACQIRIS;
using namespace std;
using namespace std::tr1;

std::tr1::shared_ptr<DetectorAnalyzerBackend> DetectorAnalyzerBackend::instance(const DetectorAnalyzerType& type)
{
  std::tr1::shared_ptr<DetectorAnalyzerBackend> detanal;
  switch(type)
  {
  case DelaylineSimple:
    detanal = shared_pointer(new DelaylineDetectorAnalyzerSimple());
    break;
  case NonSorting:
    detanal = shared_pointer(new DelaylineNonSorting());
    break;
#ifdef ACHIMSRESORTER
  case AchimsRoutine:
    detanal = shared_pointer(new HexSorter());
    break;
#endif
  default:
    throw std::invalid_argument("DetectorAnalyzerBackend::instance(): the requested type '" +
                                toString(type) + "' is unknown");
    break;
  }
  return detanal;
}

// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class implemetentation
 *                                   for all detector analyzers.
 *
 * @author Lutz Foucar
 */

#include "detector_analyzer_backend.h"
#include "delayline_detector_analyzer_simple.h"

using namespace cass::ACQIRIS;

DetectorAnalyzerBackend* DetectorAnalyzerBackend::instance(const DetectorAnalyzerType& type)
{
  DetectorAnalyzerBackend* detanal(0);
  switch(type)
  {
  case DelaylineSimple:
    detanal = new DelaylineDetectorAnalyzerSimple();
    break;
  default:
    throw std::invalid_argument("DetectorAnalyzerBackend::instance(): the requested type is unknown");
    break;
  }
  return detanal;
}

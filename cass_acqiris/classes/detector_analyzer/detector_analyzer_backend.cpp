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

std::auto_ptr<DetectorAnalyzerBackend> DetectorAnalyzerBackend::instance(const DetectorAnalyzerType& type)
{
  std::auto_ptr<DetectorAnalyzerBackend> detanal;
  switch(type)
  {
  case DelaylineSimple:
    detanal = std::auto_ptr<DetectorAnalyzerBackend>(new DelaylineDetectorAnalyzerSimple());
    break;
  default:
    throw std::invalid_argument("DetectorAnalyzerBackend::instance(): the requested type is unknown");
    break;
  }
  return detanal;
}

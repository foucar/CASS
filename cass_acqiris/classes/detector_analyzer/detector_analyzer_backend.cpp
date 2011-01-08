// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class implemetentation
 *                                   for all detector analyzers.
 *
 * @author Lutz Foucar
 */

#include "detector_analyzer_backend.h"

using namespace cass::ACQIRIS;

DetectorAnalyzerBackend* DetectorAnalyzerBackend::instance(const DetectorAnalyzerType)
{
  DetectorAnalyzerBackend* detanal(0);

  return detanal;
}

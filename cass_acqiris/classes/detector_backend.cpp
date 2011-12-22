// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_backend.cpp contains the base class definition for all detectors
 *                            that are attached to an acqiris device.
 *
 * @author Lutz Foucar
 */

#include "detector_backend.h"
#include "delayline_detector.h"

using namespace cass::ACQIRIS;
using namespace std;

DetectorBackend::shared_pointer DetectorBackend::instance(const DetectorType &dettype, const std::string &detname)
{
  shared_pointer det;

  switch(dettype)
  {
  case Delayline:
    {
      det = shared_pointer(new DelaylineDetector(detname));
    }
    break;
  case ToF:
    {
      det = shared_pointer(new TofDetector(detname));
    }
    break;
  default:
      throw invalid_argument("DetectorBackend::instance: no such detector type is present");
  }
  return det;
}

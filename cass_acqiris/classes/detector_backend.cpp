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

DetectorBackend *DetectorBackend::instance(const DetectorType &dettype, const std::string &detname)
{
  DetectorBackend *det(0);
  switch(dettype)
  {
  case Delayline:
    {
      det = new DelaylineDetector(detname);
    }
    break;
  case ToF:
    {
      det = new TofDetector(detname);
    }
    break;
  default: throw std::invalid_argument("HelperAcqirisDetectors::constructor: no such detector type is present");
  }
  return det;
}

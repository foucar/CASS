// Copyright (C) 2010 Lutz Foucar

/**
 * @file detector_analyzer_backend.h file contains base class implemetentation
 *                                   for all detector analyzers.
 *
 * @author Lutz Foucar
 */

#include <sstream>

#include "detector_analyzer_backend.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_non_sorting.h"

using namespace cass::ACQIRIS;
using namespace std;
using namespace std::tr1;
shared_ptr<DetectorAnalyzerBackend> DetectorAnalyzerBackend::instance(const DetectorAnalyzerType& type)
{
  shared_ptr<DetectorAnalyzerBackend> detanal;
  switch(type)
  {
  case DelaylineSimple:
    detanal = shared_pointer(new DelaylineDetectorAnalyzerSimple());
    break;
  case NonSorting:
      detanal = shared_pointer(new DelaylineNonSorting());
      break;
  default:
    {
      stringstream ss;
      ss <<"DetectorAnalyzerBackend::instance(): the requested type '"<<type<<"' is unknown";
      throw std::invalid_argument(ss.str());
    }
    break;
  }
  return detanal;
}

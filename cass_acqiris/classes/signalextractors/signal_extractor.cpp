//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_extractor.cpp file contains base class for all classes that
 *                            extract signals from the recorded data
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <sstream>

#include "signal_extractor.h"
#include "cfd.h"
#include "com.h"
#include "tdc_extractor.h"

using namespace cass::ACQIRIS;
using namespace std;
using namespace std::tr1;
shared_ptr<SignalExtractor> cass::ACQIRIS::SignalExtractor::instance(SignalExtractorType type)
{
  shared_ptr<SignalExtractor> sigextr;
  switch(type)
  {
  case com8:
    sigextr = shared_ptr<SignalExtractor>(new CoM8Bit());
    break;
  case com16:
    sigextr = shared_ptr<SignalExtractor>(new CoM16Bit());
    break;
  case cfd8:
    sigextr = shared_ptr<SignalExtractor>(new CFD8Bit());
    break;
  case cfd16:
    sigextr = shared_ptr<SignalExtractor>(new CFD16Bit());
    break;
  case tdcextractor:
    sigextr = shared_ptr<SignalExtractor>(new ACQIRISTDC::TDCExtractor());
    break;
  default:
    {
      stringstream ss;
      ss<<"SignalExtractor::instance: SignalExtractor type '"<<type<<"'not available";
      throw invalid_argument(ss.str());
    }
    break;
  }
  return sigextr;
}

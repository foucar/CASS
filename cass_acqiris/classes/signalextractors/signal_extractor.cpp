//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_extractor.cpp file contains base class for all classes that
 *                            extract signals from the recorded data
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "signal_extractor.h"
#include "cfd.h"
#include "com.h"

cass::ACQIRIS::SignalExtractor* cass::ACQIRIS::SignalExtractor::instance(SignalExtractorType type)
{
  SignalExtractor *sigextr(0);
  switch(type)
  {
  case com8:
    sigextr = new CoM8Bit();
    break;
  case com16:
    sigextr = new CoM16Bit();
    break;
  case cfd8:
    sigextr = new CFD8Bit();
    break;
  case cfd16:
    sigextr = new CFD16Bit();
    break;
  default:
    throw std::invalid_argument("SignalExtractor::instance: no such SignalExtractor type");
  }
  return sigextr;
}

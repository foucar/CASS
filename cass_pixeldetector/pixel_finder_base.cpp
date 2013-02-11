// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.cpp file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "pixel_finder_base.h"

#include "above_noise_finder.h"
#include "pixel_finder_simple.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

PixelFinderBase::shared_pointer PixelFinderBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "aboveNoise")
    ptr = shared_pointer(new AboveNoiseFinder());
  else if (type == "aboveNoiseAdvanced")
    ptr = shared_pointer(new AdvancedAboveNoiseFinder());
  else if (type == "simple")
    ptr = shared_pointer(new PixelFinderSimple());
  else if (type == "range")
    ptr = shared_pointer(new WithinRange());
  else if (type == "simpleMoreOptions")
    ptr = shared_pointer(new PixelFinderSimpleMoreOptions());
  else
    throw invalid_argument("PixelFinderBase::instance: Pixel Finder type '" +type +
                           "' is unknown.");
  return ptr;
}

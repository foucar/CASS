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
  else if (type == "pixfind")
    ptr = shared_pointer(new PixelFinderSimple());
  else
  {
    /** @todo get rid of stringstream */
    stringstream ss;
    ss << "PixelFinderBase::instance: Pixel Finder type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

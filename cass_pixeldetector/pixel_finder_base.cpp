// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.cpp file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "pixel_finder_base.h"


using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

PixelFinderBase::shared_pointer PixelFinderBase::instance(const string &type)
{
  shared_pointer ptr;
//  if (type == "aboveNoise")
//    ptr = shared_pointer(new AboveNoise());
//  else if (type == "pixfind")
//    ptr = shared_pointer(new PixelFinder());
//  else
  {
    stringstream ss;
    ss << "PixelFinderBase::instance: Pixel Finder type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

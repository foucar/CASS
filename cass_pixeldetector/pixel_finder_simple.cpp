// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_finder_simple.cpp contains pixel finder that works like Per Johnsons
 *
 * @author Lutz Foucar
 */

#include "pixel_finder_simple.h"

#include "cass_settings.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

PixelFinderSimple::PixelFinderSimple()
{

}

PixelFinderSimple::pixels_t& PixelFinderSimple::operator ()(const Frame &frame, pixels_t &pixels)
{
 return pixels;
}

void PixelFinderSimple::loadSettings(CASSSettings &s)
{

}

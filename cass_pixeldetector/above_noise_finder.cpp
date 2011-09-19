// Copyright (C) 2011 Lutz Foucar

/**
 * @file above_noise_finder.cpp contains hll like pixel finder using noise maps
 *
 * @author Lutz Foucar
 */

#include "above_noise_finder.h"

#include "cass_settings.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

AboveNoiseFinder::AboveNoiseFinder()
{

}

AboveNoiseFinder::pixels_t& AboveNoiseFinder::operator ()(const Frame &frame, pixels_t &pixels)
{
 return pixels;
}

void AboveNoiseFinder::loadSettings(CASSSettings &s)
{

}

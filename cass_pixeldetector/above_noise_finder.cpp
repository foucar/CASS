// Copyright (C) 2011 Lutz Foucar

/**
 * @file hll_frame_processor.cpp contains hll correctionlike frame processor.
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

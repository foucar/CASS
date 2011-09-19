// Copyright (C) 2011 Lutz Foucar

/**
 * @file hll_frame_processor.cpp contains hll correctionlike frame processor.
 *
 * @author Lutz Foucar
 */

#include "hll_frame_processor.h"

#include "cass_settings.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

HLLProcessor::HLLProcessor()
{

}

Frame& HLLProcessor::operator ()(Frame &frame)
{
 return frame;
}

void HLLProcessor::loadSettings(CASSSettings &s)
{

}

// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.cpp contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#include "commonmode_calculators.h"

using namespace cass;
using namespace pixeldetector;
using namespace commonmode;
using namespace std;
using namespace std::tr1;

MeanCalculator::MeanCalculator()
{}

pixel_t MeanCalculator::operator ()(frame_t::const_iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  return commonmodelevel;
}

void MeanCalculator::loadSettings(CASSSettings &s)
{

}

MedianCalculator::MedianCalculator()
{}

pixel_t MedianCalculator::operator ()(frame_t::const_iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  return commonmodelevel;
}

void MedianCalculator::loadSettings(CASSSettings &s)
{

}

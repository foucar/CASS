// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.cpp contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#include <set>
#include <numeric>

#include "commonmode_calculators.h"

#include "common_data.h"

using namespace cass;
using namespace pixeldetector;
using namespace commonmode;
using namespace std;
using namespace std::tr1;

MeanCalculator::MeanCalculator()
{}

pixel_t MeanCalculator::operator ()(frame_t::iterator &pixel, size_t idx)const
{
  typedef multiset<pixel_t> orderedPixels_t;

  pixel_t commonmodelevel(0);
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator offset(_commondata->offsetMap.begin()+idx);
  frame_t::const_iterator noise(_commondata->noiseMap.begin()+idx);
  orderedPixels_t pixels;
  for(size_t i(0); i<_nbrPixels;++i,++pixel,++offset,++noise)
  {
    pixel_t offsetcorrectedPixel(*pixel - *offset);
    if(offsetcorrectedPixel < _multiplier * *noise)
    {
      pixels.insert(offsetcorrectedPixel);
    }
  }
  orderedPixels_t::iterator begin(pixels.begin());
  orderedPixels_t::iterator end(pixels.end());
  advance(begin,_nbrElementsToRemove.first);
  advance(end,-1*(_nbrElementsToRemove.second));
  commonmodelevel = (_minNbrPixels < distance(begin,end))? accumulate(begin,end,0) / distance(begin,end) : 0;

  return commonmodelevel;
}

void MeanCalculator::loadSettings(CASSSettings &s)
{
  load(s);
}

MedianCalculator::MedianCalculator()
{}

pixel_t MedianCalculator::operator ()(frame_t::iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  return commonmodelevel;
}

void MedianCalculator::loadSettings(CASSSettings &s)
{
  load(s);
}

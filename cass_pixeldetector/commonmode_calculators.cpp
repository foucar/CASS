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

namespace cass
{
namespace pixeldetector
{
namespace commonmode
{
typedef multiset<pixel_t> orderedPixels_t;

/** build up the list of pixels that contribute to the common mode calculation
 *
 * go as much pixels as told via nbrPixels. Offset correct each pixel then
 * check whether pixel minus the initial common mode level is less than the noise
 * if so add it to the ordered list of pixels.
 *
 * @param nbrPixels the number of pixel that cover the range to check for the
 *                  common mode level.
 * @param pixel iterator to the point in the frame that should be checked for
 *              the common mode level.
 * @param offset const_iterator that starts at the position in the offset map
 *               that we are investigating
 * @param noise const_iterator that starts at the position in the noise map that
 *              we are investigating
 * @param multiplier the mulitplier to multiply to the noise value
 * @param initialLevel the inital level of the common mode
 * @param[out] pixels The list of pixels found in this function
 *
 * @author Lutz Foucar
 */
void createPixelList(size_t nbrPixels,
                     frame_t::iterator pixel,
                     frame_t::const_iterator offset,
                     frame_t::const_iterator noise,
                     float multiplier,
                     pixel_t initialLevel,
                     orderedPixels_t& pixels)
{
  for(size_t i(0); i<nbrPixels;++i,++pixel,++offset,++noise)
  {
    pixel_t offsetcorrectedPixel(*pixel - *offset );
    if((offsetcorrectedPixel - initialLevel) < (multiplier * *noise))
    {
      pixels.insert(offsetcorrectedPixel);
    }
  }
}
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass


MeanCalculator::MeanCalculator()
{}

pixel_t MeanCalculator::operator ()(frame_t::iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator offset(_commondata->offsetMap.begin()+idx);
  frame_t::const_iterator noise(_commondata->noiseMap.begin()+idx);
  orderedPixels_t pixels;
  createPixelList(_nbrPixels,
                  pixel,
                  offset,
                  noise,
                  _multiplier,
                  0.,
                  pixels);
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

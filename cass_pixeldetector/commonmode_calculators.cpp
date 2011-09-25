// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.cpp contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#include <vector>
#include <numeric>
#include <algorithm>

#include "commonmode_calculators.h"

#include "common_data.h"
#include "cass_settings.h"

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
typedef vector<pixel_t> pixels_t;

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
                     pixels_t& pixels)
{
  for(size_t i(0); i<nbrPixels;++i,++pixel,++offset,++noise)
  {
    pixel_t offsetcorrectedPixel(*pixel - *offset );
    if((offsetcorrectedPixel - initialLevel) < (multiplier * *noise))
    {
      pixels.push_back(offsetcorrectedPixel);
    }
  }

}
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass


pixeldetector::pixel_t MeanCalculator::operator ()(frame_t::iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator offset(_commondata->offsetMap.begin()+idx);
  frame_t::const_iterator noise(_commondata->noiseMap.begin()+idx);
  pixels_t pixels;
  createPixelList(_nbrPixels, pixel, offset, noise, _multiplier, 0., pixels);
  const int nbrElementsOfInterest
      (pixels.size() - _nbrMinimumElementsToRemove - _nbrMaximumElementsToRemove);
  const bool shouldCalcCommonMode (_minNbrPixels <  nbrElementsOfInterest);
  if (shouldCalcCommonMode)
  {
    sort(pixels.begin(),pixels.end());
    pixels_t::iterator begin(pixels.begin());
    pixels_t::iterator end(pixels.end());
    advance(begin,_nbrMinimumElementsToRemove);
    advance(end,-1*(_nbrMaximumElementsToRemove));
    commonmodelevel = accumulate(begin,end,0) / distance(begin,end);
  }
  else
  {
    commonmodelevel = 0.;
  }
  return commonmodelevel;
}

void MeanCalculator::loadSettings(CASSSettings &s)
{
  load(s);
  s.beginGroup("MeanCommonMode");
  _nbrMaximumElementsToRemove = s.value("",5).toUInt();
  _nbrMinimumElementsToRemove = s.value("",0).toUInt();
  _minNbrPixels = s.value("",8).toUInt();
  s.endGroup();
#warning finalize implementation
}

pixeldetector::pixel_t MedianCalculator::operator ()(frame_t::iterator &pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator offset(_commondata->offsetMap.begin()+idx);
  frame_t::const_iterator noise(_commondata->noiseMap.begin()+idx);
  pixels_t pixels;
  createPixelList(_nbrPixels, pixel, offset, noise, _multiplier, 0., pixels);
  const int nbrElementsOfInterest
      (pixels.size() - _nbrDisregardedMinimumElements - _nbrDisregardedMaximumElements);
  const bool shouldCalcCommonMode (_minNbrPixels <  nbrElementsOfInterest);
  if (shouldCalcCommonMode)
  {
    size_t median = 0.5*nbrElementsOfInterest + _nbrDisregardedMinimumElements;
    nth_element(pixels.begin(),pixels.begin()+median,pixels.end());
    commonmodelevel = pixels[median];
  }
  else
  {
    commonmodelevel = 0.;
  }
  return commonmodelevel;
}

void MedianCalculator::loadSettings(CASSSettings &s)
{
  load(s);
#warning implement this

}

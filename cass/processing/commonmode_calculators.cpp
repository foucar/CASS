// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.cpp contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#include <numeric>
#include <algorithm>
#include <iostream>

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
 * @param mask const_iterator that starts at the position in the mask that we
 *             are interested in.
 * @param multiplier the mulitplier to multiply to the noise value
 * @param initialLevel the inital level of the common mode
 * @param[out] pixels The list of pixels found in this function
 *
 * @author Lutz Foucar
 */
void createPixelList(size_t nbrPixels,
                     frame_t::const_iterator pixel,
                     frame_t::const_iterator offset,
                     frame_t::const_iterator noise,
                     CommonData::mask_t::const_iterator mask,
                     float multiplier,
                     pixel_t initialLevel,
                     pixels_t& pixels)
{
  for(size_t i(0); i<nbrPixels;++i,++pixel,++offset,++noise)
  {
    if (*mask)
    {
      pixel_t offsetcorrectedPixel(*pixel - *offset );
      if((offsetcorrectedPixel - initialLevel) < (multiplier * *noise))
      {
        pixels.push_back(offsetcorrectedPixel);
      }
    }
  }

}
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass


pixeldetector::pixel_t SimpleMeanCalculator::operator ()(frame_t::const_iterator pixel, size_t idx)const
{
  frame_t::const_iterator offset(_commondata->offsetMap.begin()+idx);
  frame_t::const_iterator noise(_commondata->noiseMap.begin()+idx);
  CommonData::mask_t::const_iterator mask(_commondata->mask.begin()+idx);
  pixel_t commlvl(0);
  size_t accumulatedValues(0);
  pixel_t pixel_wo_offset(0);
  for(size_t i(0); i<_nbrPixels;++i,++pixel,++offset,++noise,++mask)
  {
    if (*mask)
    {
      pixel_wo_offset = *pixel - *offset;
      if((pixel_wo_offset) < (_multiplier * *noise))
      {
        ++accumulatedValues;
        commlvl += ((pixel_wo_offset - commlvl) / accumulatedValues);
      }
    }
  }
//  if (accumulatedValues < _minNbrPixels || qFuzzyCompare(commlvl,0.f))
//    cout << _minNbrPixels << " "<< _nbrPixels<< " " << accumulatedValues<< " " <<commlvl<<endl;
  return (_minNbrPixels <  accumulatedValues ? commlvl : 0.);
}

void SimpleMeanCalculator::loadSettings(CASSSettings &s)
{
  load(s);
  s.beginGroup("SimpleMeanCommonMode");
  _minNbrPixels = s.value("MinNbrPixels",8).toUInt();
  s.endGroup();
}


pixeldetector::pixel_t MeanCalculator::operator ()(frame_t::const_iterator pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  pixels_t pixels;
  createPixelList(_nbrPixels, pixel,
                  _commondata->offsetMap.begin()+idx,
                  _commondata->noiseMap.begin()+idx,
                  _commondata->mask.begin()+idx,
                  _multiplier, 0., pixels);
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
    commonmodelevel = accumulate(begin,end,0) / static_cast<pixel_t>(distance(begin,end));
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
  _nbrMaximumElementsToRemove = s.value("NbrMaxDisregardedValues",5).toUInt();
  _nbrMinimumElementsToRemove = s.value("NbrMinDisregardedValues",0).toUInt();
  _minNbrPixels = s.value("MinNbrPixels",8).toUInt();
  s.endGroup();
}

pixeldetector::pixel_t MedianCalculator::operator ()(frame_t::const_iterator pixel, size_t idx)const
{
  pixel_t commonmodelevel(0);
  pixels_t pixels;
  createPixelList(_nbrPixels, pixel,
                  _commondata->offsetMap.begin()+idx,
                  _commondata->noiseMap.begin()+idx,
                  _commondata->mask.begin()+idx,
                  _multiplier, 0., pixels);
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
  s.beginGroup("MedianCommonMode");
  _nbrDisregardedMaximumElements = s.value("NbrMaxDisregardedValues",5).toUInt();
  _nbrDisregardedMinimumElements = s.value("NbrMinDisregardedValues",0).toUInt();
  _minNbrPixels = s.value("MinNbrPixels",8).toUInt();
  s.endGroup();
}

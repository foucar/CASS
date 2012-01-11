// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreators.h contains all correction map creators.
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "mapcreators.h"

#include "cass_settings.h"
#include "common_data.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;


namespace cass
{
namespace pixeldetector
{
/** create the list of pixels that are non events
 *
 * create a list of all pixels from the storage. Exclude all
 * pixels that might contain an event (photon or other).
 *
 * @param offset the offset for the pixel at index pixel
 * @param noise the noise for the pixel at index pixel
 * @param storage the container for all the frames data
 * @param pixel index of the pixel that one wants to create the pixel list for.
 * @param exclude flag to show whether to exclude the pixels that potentially
 *        a photon hit
 * @param[out] pixellist the list of pixels the are non events
 *
 * @author Lutz Foucar
 */
void createPixelList(const frame_t::value_type &offset,
                     const frame_t::value_type &noise,
                     const MapCreatorBase::storage_t& storage,
                     size_t pixel,
                     bool exclude,
                     frame_t &pixellist)
{
  MapCreatorBase::storage_t::const_iterator frame(storage.begin());
  MapCreatorBase::storage_t::const_iterator frameEnd(storage.end());
  for (; frame != frameEnd ; ++frame)
  {
    if ((*frame)[pixel] - offset < noise || !exclude)
      pixellist.push_back((*frame)[pixel]);
  }
}

/** calulate the standart deviation of distribution
 *
 * @return the standart deviation
 * @param values the values to calc the standart deviation from the mean value
 * @param mean the mean value to calc the standart deviation from
 *
 * @author Lutz Foucar
 */
frame_t::value_type calcNoise(const frame_t& values, const frame_t::value_type& mean)
{
  frame_t zero_mean(values);
  transform( zero_mean.begin(), zero_mean.end(),
             zero_mean.begin(),bind2nd( minus<frame_t::value_type>(), mean ) );

  frame_t::value_type deviation
      (inner_product( zero_mean.begin(),zero_mean.end(), zero_mean.begin(), 0.0f ));
  deviation = sqrt( deviation / ( values.size() - 1 ) );
  return deviation;
}

/** calculate the mean of the distribution
 *
 * before calulating the mean value of the distribution remove the nbr of elements
 * that have the lowest values and the nbr of elements with the hightest values.
 *
 * @return the mean value
 * @param values the values to calculate the mean from
 * @param mindisregard the number of minimum values to disregard
 * @param maxdisregard the number of maximum values to disregard
 *
 * @author Lutz Foucar
 */
frame_t::value_type calcMean(frame_t& values, size_t mindisregard, size_t maxdisregard)
{
  sort(values.begin(),values.end());
  frame_t::iterator begin(values.begin());
  frame_t::iterator end(values.end());
  advance(begin,mindisregard);
  advance(end,-1*(maxdisregard));
  return (accumulate(begin,end,0) / static_cast<frame_t::value_type>(distance(begin,end)));
}

/** calculate the median of the distribution
 *
 * before calulating the median value of the distribution remove the nbr of
 * elements that have the lowest values and the nbr of elements with the
 * hightest values.
 *
 * @return the median value
 * @param values the values to calculate the median from
 * @param mindisregard the number of minimum values to disregard
 * @param maxdisregard the number of maximum values to disregard
 *
 * @author Lutz Foucar
 */
frame_t::value_type calcMedian(frame_t& values,
                               size_t mindisregard,
                               size_t maxdisregard)
{
  const int nbrElementsOfInterest
      (values.size() - mindisregard - maxdisregard);
  size_t median = 0.5*nbrElementsOfInterest + mindisregard;
  nth_element(values.begin(),values.begin()+median,values.end());
  return (values[median]);
}
}//end namespace pixeldetector
}//end namespace cass


void NonAlteringMaps::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  _commondata = CommonData::instance(detectorname);
  QWriteLocker lock(&_commondata->lock);
  frame_t::iterator offset(_commondata->offsetMap.begin());
  for (;offset != _commondata->offsetMap.end(); ++offset)
    *offset = 0;
  frame_t::iterator noise(_commondata->noiseMap.begin());
  for (;noise != _commondata->noiseMap.end(); ++noise)
    *noise = 0;
  frame_t::iterator corval(_commondata->correctionMap.begin());
  for (;corval != _commondata->correctionMap.end(); ++corval)
    *corval = 1;
}


void FixedMaps::operator ()(const Frame &frame)
{
  if(!_createMaps)
    return;
  else
  {
    if (_storage.size() < _nbrFrames)
      _storage.push_back(frame.data);
    else
    {
      frame_t pixels;
      frame_t::iterator offset(_commondata->offsetMap.begin());
      frame_t::iterator offsetEnd(_commondata->offsetMap.end());
      frame_t::iterator noise(_commondata->noiseMap.begin());
      size_t idx(0);
      for (;offset != offsetEnd; ++offset, ++noise, ++idx)
      {
        pixels.clear();
        for (size_t i=0; i < 2; ++i)
        {
          createPixelList(*offset,*noise,_storage,idx,i,pixels);
          if(!pixels.empty())
          {
            *offset = _calcOffset(pixels,_minDisregarded,_maxDisregarded);
            *noise = calcNoise(pixels, *offset);
          }
        }
        _createMaps = false;
        _storage.clear();
        _commondata->saveMaps();
        _commondata->createCorMap();
      }
    }
  }
}

void FixedMaps::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("FixedCreator");
  _commondata = CommonData::instance(detectorname);
  _nbrFrames = s.value("NbrFrames",200).toUInt();
  _maxDisregarded = s.value("DisregardedHighValues",5).toUInt();
  _minDisregarded = s.value("DisregardedLowValues",0).toUInt();
  _createMaps = s.value("StartInstantly",false).toBool();
  _writeMaps = s.value("WriteMaps",true).toBool();
  if(s.value("UseMedian",false).toBool())
    _calcOffset = &calcMedian;
  else
    _calcOffset = &calcMean;
  s.endGroup();
}


void MovingMaps::operator ()(const Frame &frame)
{
#warning "implement this (not urgent)"
  throw runtime_error("MovingMaps() has not been implemented yet");

}

void MovingMaps::loadSettings(CASSSettings &s)
{
#warning "implement this (not urgent)"
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  _commondata = CommonData::instance(detectorname);
  throw runtime_error("MovingMaps::loadSettings() has not been implemented yet");

}

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

#include <QtCore/QStringList>

#include "gaincalibration.h"

#include "cass_settings.h"
#include "common_data.h"
#include "advanced_pixeldetector.h"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;


namespace cass
{
namespace pixeldetector
{
}//end namespace pixeldetector
}//end namespace cass




void GainCalibration::generateCalibration(const Frame &frame)
{
  QWriteLocker (&_commondata->lock);

  _statistics.resize(frame.data.size(),make_pair(0,0.));
  vector<statistics_t>::iterator stat(_statistics.begin());
  vector<statistics_t>::const_iterator statEnd(_statistics.end());

  frame_t::const_iterator pixel(frame.data.begin());

  /** average pixelvalues per pixel that lie within the given adu range */
  while (stat != statEnd)
  {
    const float pixval(*pixel);

    if (pixval < _range.first  ||  _range.second < pixval)
      continue;

    double &ave((*stat).second);
    const double N(static_cast<double>(++(*stat).first));
    ave = ave + 1./N*(pixval - ave);

    ++stat;
    ++pixel;
  }

  /** check the median nbr of photons per pixel. Use it as criteria whether to
   *  generate the gain map.
   */
  vector<statistics_t> statcpy(_statistics);
  const size_t medianPos(0.5*statcpy.size());
  nth_element(statcpy.begin(),statcpy.begin() + medianPos, statcpy.end(),
              bind(less<statistics_t::first_type>(),
                   bind<statistics_t::first_type>(&statistics_t::first,_1),
                   bind<statistics_t::first_type>(&statistics_t::first,_2)));
  if (statcpy[medianPos].first < _minMedianCounts)
    return;

  /** calculate the average of the average pixelvalues, disregarding pixels
   *  that have not seen enough photons in the right ADU range.
   */
  int count(0);
  double ave(0);
  stat = _statistics.begin();
  while (stat != statEnd)
  {
    const statistics_t &s(*stat++);
    if (s.first < _minPhotonCount)
      continue;
    ++count;
    const double N(static_cast<double>(++(*stat).first));
    ave = ave + 1./N*(s.second - ave);
  }

  /** assing the gain value for each pixel that has seen enough statistics.
   *  gain is calculated by formula
   *  \f$ gain = frac{average_average_pixelvalue}{average_pixelvalue} \f$
   *  If not enough photons are in the pixel, set the predefined user value
   */
  frame_t::iterator gain(_commondata->gain_cteMap.begin());
  stat = _statistics.begin();
  while (stat != statEnd)
  {
    const statistics_t &s(*stat++);
    *gain++ = (s.first < _minPhotonCount) ? _constGain : ave/s.second;
  }

  /** if requested write the gain map to file, reset the calibration */
  if (_writeFile)
    _commondata->saveGainMap();
  fill(_statistics.begin(),_statistics.end(),make_pair(0,0.));
  _createMap = bind(&GainCalibration::doNothing,this,_1);
}

void GainCalibration::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("GainFixedADURange");
  _commondata = CommonData::instance(detectorname);
  _minPhotonCount = s.value("MinimumPhotonCount",50).toInt();
  _minMedianCounts = s.value("MinimumMedianCounts",200).toInt();
  _range = make_pair(s.value("MinADURange",0).toUInt(),
                     s.value("MaxADURange",1000).toUInt());
  _writeFile = s.value("SaveCalibration",true).toBool();
  if (s.value("StartInstantly",false).toBool())
    _createMap = bind(&GainCalibration::generateCalibration,this,_1);
  else
    _createMap = bind(&GainCalibration::doNothing,this,_1);
}


void GainCalibration::controlCalibration(const string &/*unused*/)
{
  Log::add(Log::INFO,"GainCalibration::controlCalibration(): start collecting statistics for gain calibration'");
  _createMap = bind(&GainCalibration::generateCalibration,this,_1);
}


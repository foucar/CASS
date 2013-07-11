// Copyright (C) 2013 Lutz Foucar

/**
 * @file gaincalibration.cpp contains a gain calibration functor
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>

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


void GainCalibration::generateCalibration(const Frame &frame)
{
  QWriteLocker (&_commondata->lock);

  ++_counter;
  _statistics.resize(frame.data.size(),make_pair(0,0.));
  vector<statistics_t>::iterator stat(_statistics.begin());
  vector<statistics_t>::const_iterator statEnd(_statistics.end());

  frame_t::const_iterator pixel(frame.data.begin());
  frame_t::const_iterator offset(_commondata->offsetMap.begin());

  /** average the common mode and offset corrected pixelvalues per pixel that
   *  lie within the given adu range
   */
  const commonmode::CalculatorBase &commonMode(*_commonModeCalculator);
  const size_t length(commonMode.width());
  const size_t parts(frame.data.size() / length);
  size_t idx(0);
  for (size_t part(0); part < parts; ++part)
  {
    const pixel_t cmode(commonMode(pixel,idx));
    for (size_t i(0); i < length; ++i, ++stat, ++pixel, ++idx)
    {
      const pixel_t pixval(*pixel - *offset++ - cmode);

      if (pixval < _range.first  ||  _range.second < pixval)
        continue;

      double &ave((*stat).second);
      const double N(static_cast<double>(++(*stat).first));
      ave = ave + (pixval - ave)/N;
    }
  }

  /** check the median nbr of photons per pixel. Use this or the number of 
   *  frames processed so far as criteria whether to generate the gain map.
   */
  vector<statistics_t> statcpy(_statistics);
  const size_t medianPos(0.5*statcpy.size());
  nth_element(statcpy.begin(),statcpy.begin() + medianPos, statcpy.end(),
              bind(less<statistics_t::first_type>(),
                   bind<statistics_t::first_type>(&statistics_t::first,_1),
                   bind<statistics_t::first_type>(&statistics_t::first,_2)));
  if (statcpy[medianPos].first < _minMedianCounts || _counter == _nFrames)
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
    ave = ave + (s.second - ave)/count;
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
  string detectorname(DetectorName::fromSettings(s));
  s.beginGroup("GainFixedADURange");
  _commondata = CommonData::instance(detectorname);
  _minPhotonCount = s.value("MinimumPhotonCount",50).toInt();
  _minMedianCounts = s.value("MinimumMedianCounts",200).toInt();
  _range = make_pair(s.value("MinADURange",0).toUInt(),
                     s.value("MaxADURange",1000).toUInt());
  _writeFile = s.value("SaveCalibration",true).toBool();
  _counter = 0;
  _nFrames = s.value("NbrFrames",-1).toInt();
  if (s.value("StartInstantly",false).toBool())
    _createMap = bind(&GainCalibration::generateCalibration,this,_1);
  else
    _createMap = bind(&GainCalibration::doNothing,this,_1);
  string commonmodetype (s.value("CommonModeCalculationType","none").toString().toStdString());
  _commonModeCalculator = commonmode::CalculatorBase::instance(commonmodetype);
  _commonModeCalculator->loadSettings(s);
  s.endGroup();
}


void GainCalibration::controlCalibration(const string &/*unused*/)
{
  Log::add(Log::INFO,"GainCalibration::controlCalibration(): start collecting statistics for gain calibration'");
  _counter=0;
  _createMap = bind(&GainCalibration::generateCalibration,this,_1);
}


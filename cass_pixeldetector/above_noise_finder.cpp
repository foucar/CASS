// Copyright (C) 2011 Lutz Foucar

/**
 * @file above_noise_finder.cpp contains hll like pixel finder using noise maps
 *
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "above_noise_finder.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

AboveNoiseFinder::AboveNoiseFinder()
{}

AboveNoiseFinder::pixels_t& AboveNoiseFinder::operator ()(const Frame &frame, pixels_t &pixels)
{
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator pixel(frame.data.begin());
  frame_t::const_iterator noise(_commondata->noiseMap.begin());
  size_t idx(0);
  for (; pixel != frame.data.end(); ++pixel, ++noise, ++idx)
  {
    if(*noise * _multiplier < *pixel)
    {
      const uint16_t x(idx % frame.columns);
      const uint16_t y(idx / frame.columns);
      pixels.push_back(Pixel(x,y,*pixel));
    }
  }
  return pixels;
}

void AboveNoiseFinder::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").back().toStdString());
  _commondata = CommonData::instance(detectorname);
  s.beginGroup("AboveNoiseFinder");
  _multiplier = s.value("Multiplier",4.).toFloat();
  s.endGroup();
}

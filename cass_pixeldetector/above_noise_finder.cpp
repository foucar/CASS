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


AdvancedAboveNoiseFinder::AdvancedAboveNoiseFinder()
{}

AdvancedAboveNoiseFinder::pixels_t& AdvancedAboveNoiseFinder::operator ()(const Frame &frame, pixels_t &pixels)
{
  QReadLocker lock(&_commondata->lock);
  frame_t::const_iterator pixel(frame.data.begin());
  frame_t::const_iterator noise(_commondata->noiseMap.begin());
  size_t idx(0);
  vector<float> box;
  const uint16_t ncols(frame.columns);
  const uint16_t nrows(frame.rows);
  for (; pixel != frame.data.end(); ++pixel, ++noise, ++idx)
  {
    //    if(*noise * _multiplier < *pixel)
    if (_threshold < *pixel)
    {
      const uint16_t x(idx % ncols);
      const uint16_t y(idx / ncols);

      const uint16_t xboxbegin(max(static_cast<int>(0),static_cast<int>(x)-static_cast<int>(_boxSize.first)));
      const uint16_t xboxend(min(ncols,static_cast<uint16_t>(x+_boxSize.first)));
      const uint16_t yboxbegin(max(static_cast<int>(0),static_cast<int>(y)-static_cast<int>(_boxSize.second)));
      const uint16_t yboxend(min(nrows,static_cast<uint16_t>(y+_boxSize.second)));

      box.clear();
      for (size_t yb=yboxbegin; yb<yboxend;++yb)
      {
        for (size_t xb=xboxbegin; xb<xboxend;++xb)
        {
          const size_t pixAddrBox(yb*frame.columns+xb);
          const float pixel_box(frame.data[pixAddrBox]);
          /** check if current sourrounding pixel is a bad pixel (0.),
           *  if so we should disregard the pixel as a candiate and check the
           *  next pixel.
           */
          if (qFuzzyCompare(pixel_box,0.f) )
            goto NEXTPIXEL;
          else
            box.push_back(pixel_box);
        }
      }

      if (box.size() > 1)
      {
        const size_t mid(0.5*box.size());
        nth_element(box.begin(), box.begin() + mid, box.end());
        const float bckgnd = box[mid];
        const float clrdpixel(*pixel - bckgnd);
        if (_threshold < clrdpixel)
          pixels.push_back(Pixel(x,y,clrdpixel));
      }
NEXTPIXEL:
      ;
    }
  }
  return pixels;
}

void AdvancedAboveNoiseFinder::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").back().toStdString());
  _commondata = CommonData::instance(detectorname);
  s.beginGroup("AdvancedAboveNoiseFinder");
  _multiplier = s.value("Multiplier",4.).toFloat();
  _threshold = s.value("Threshold",10).toFloat();
  _boxSize = make_pair(s.value("BoxSizeX", 10).toUInt(),
                       s.value("BoxSizeY",10).toUInt());

  s.endGroup();
}

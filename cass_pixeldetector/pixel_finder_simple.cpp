// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_finder_simple.cpp contains pixel finder that works like Per Johnsons
 *
 * @author Lutz Foucar
 */

#include "pixel_finder_simple.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

PixelFinderSimple::PixelFinderSimple()
{

}

PixelFinderSimple::pixels_t& PixelFinderSimple::operator ()(const Frame &frame, pixels_t &pixels)
{
  size_t idx(0);
  frame_t::const_iterator pixel(frame.data.begin());
  for (;pixel != frame.data.end(); ++pixel)
  {
    ++idx;
    const uint16_t x(idx % frame.columns);
    const uint16_t y(idx / frame.columns);
    if (*pixel > _threshold &&
        //check wether point is not at an edge
        y > 0 &&
        y < frame.rows-1 &&
        x > 0 &&
        x < frame.columns+1 &&
        // Check all surrounding pixels
        frame.data[idx-frame.columns-1] < *pixel && //upper left
        frame.data[idx-frame.columns]   < *pixel && //upper middle
        frame.data[idx-frame.columns+1] < *pixel && //upper right
        frame.data[idx-1]               < *pixel && //left
        frame.data[idx+1]               < *pixel && //right
        frame.data[idx+frame.columns-1] < *pixel && //lower left
        frame.data[idx+frame.columns]   < *pixel && //lower middle
        frame.data[idx+frame.columns+1] < *pixel)   //lower right
    {
      pixels.push_back(Pixel(x,y,*pixel));
    }
  }
  return pixels;
}

void PixelFinderSimple::loadSettings(CASSSettings &s)
{
  s.beginGroup("SimpleFinder");
  _threshold = s.value("Threshold",0).toUInt();
  s.endGroup();
}

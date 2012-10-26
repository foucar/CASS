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


PixelFinderSimpleMoreOptions::PixelFinderSimpleMoreOptions()
{

}

PixelFinderSimple::pixels_t& PixelFinderSimpleMoreOptions::operator ()(const Frame &frame, pixels_t &pixels)
{
  size_t idx(0);
  frame_t::const_iterator pixel(frame.data.begin());
  for (;pixel != frame.data.end(); ++pixel)
  {
    ++idx;
    const uint16_t x(idx % frame.columns);
    const uint16_t y(idx / frame.columns);
    //not at edges
    if (*pixel > _threshold &&
        y > _squaresize-1 &&
        y < frame.rows-_squaresize &&
        x > _squaresize-1 &&
        y < frame.columns-_squaresize)
    {
      //check surrounding pixels
      bool pixelIsLocalMaximum(true);
      for (int squareRow=-_squaresize; squareRow <= _squaresize; ++squareRow)
      {
        for (int squareCol=-_squaresize; squareCol <= _squaresize; ++squareCol)
        {
          if (!(squareRow == 0 && squareCol == 0))
            pixelIsLocalMaximum = pixelIsLocalMaximum && (frame.data[idx + squareRow*frame.columns + squareCol] < *pixel);
        }
      }
      if (pixelIsLocalMaximum)
      {
        pixels.push_back(Pixel(x,y,*pixel));
      }
    }
  }
  return pixels;
}

void PixelFinderSimpleMoreOptions::loadSettings(CASSSettings &s)
{
  s.beginGroup("SimpleFinder");
  _threshold = s.value("Threshold",0).toUInt();
  _squaresize = s.value("SquareSize",1).toUInt();
  s.endGroup();
}



WithinRange::WithinRange()
{}

WithinRange::pixels_t& WithinRange::operator ()(const Frame &frame, pixels_t &pixels)
{
  frame_t::const_iterator pixel(frame.data.begin());
  frame_t::const_iterator end(frame.data.end());
  size_t idx(0);
  for (; pixel != end; ++pixel, ++idx)
  {
//    cout << *pixel<<" " << _range.first<<" "<< _range.second<<" "<< (_range.first < *pixel && *pixel < _range.second) << endl;
    if(_range.first < *pixel && *pixel < _range.second)
    {
      const uint16_t x(idx % frame.columns);
      const uint16_t y(idx / frame.columns);
      pixels.push_back(Pixel(x,y,*pixel));
    }
  }
  return pixels;
}

void WithinRange::loadSettings(CASSSettings &s)
{
  s.beginGroup("InRangeFinder");
  _range = make_pair(s.value("LowerThreshold",0).toFloat(),
                     s.value("UpperThreshold",1e6).toFloat());
  s.endGroup();
}

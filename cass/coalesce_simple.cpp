// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalesce_simple.cpp contains class that does the pixel coalescing in a
 *                           simple way.
 *
 * @author Lutz Foucar
 */

#include "coalesce_simple.h"

#include "cass_settings.h"

using namespace cass;
using namespace std;

namespace cass
{
  /** check if pixel is neighbour
   *
   * @author Lutz Foucar
   */
  struct isNeighbour
  {
    /** the coordinate of the neighbour to check */
    pair<uint16_t,uint16_t> _neighbourCoordinate;

    /** constructor
     *
     * @param coordinate the coordinate of the neighbour
     */
    isNeighbour(const pair<uint16_t,uint16_t> & coordinate)
      :_neighbourCoordinate(coordinate)
    {}

    /** the operator
     *
     * @return true when pixel has not been used and is neighbour
     * @param pixel reference to the potential neighbour
     */
    bool operator()(const Pixel &pixel)
    {
      return (!pixel.isUsed() &&
              pixel.x() == _neighbourCoordinate.first &&
              pixel.y() == _neighbourCoordinate.second);
    }
  };

  /** find all neighbours of a pixel
   *
   * @author Lutz Foucar
   */
  void findNeighbours(const Pixel& pixel, PixelDetector::pixelList_t &coalescedpixellist)
  {
    pair<uint16_t,uint16_t> pixelCoordinate(make_pair(pixel.x(),pixel.y()));
  }

  /** coalesce one pixel from the list
   *
   * @author Lutz Foucar
   */
  Pixel coalesce(const PixelDetector::pixelList_t &pixellist)
  {
    Pixel pixel;
    return pixel;
  }
}

SimpleCoalesce::SimpleCoalesce()
{}

void SimpleCoalesce::loadSettings()
{
}

PixelDetector::pixelList_t& SimpleCoalesce::operator() (const PixelDetector::pixelList_t &pixellist,
                                                        PixelDetector::pixelList_t &coalescedpixels)
{
  PixelDetector::pixelList_t::const_iterator pixel(pixellist.begin());
  for(; pixel != pixellist.end();++ pixel)
  {
    if (!pixel->isUsed())
    {
      PixelDetector::pixelList_t coalescedpixellist;
      findNeighbours(*pixel,coalescedpixellist);
      Pixel coalescedpixel(coalesce(coalescedpixellist));
      coalescedpixels.push_back(coalescedpixel);
    }
  }
  return (coalescedpixels);
}

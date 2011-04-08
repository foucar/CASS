// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalesce_simple.cpp contains class that does the pixel coalescing in a
 *                           simple way.
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include "coalesce_simple.h"

#include "cass_settings.h"

using namespace cass;
using namespace std;

namespace cass
{
  namespace Direction
  {
    /** enum for easier code */
    enum direction{origin, north, east, south, west};
  }

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
  void findNeighbours(uint16_t depth,
                      Pixel& pixel,
                      Direction::direction direction,
                      PixelDetector::pixelList_t &pixellist,
                      PixelDetector::pixelList_t &coalescedpixellist)
  {
    if (depth > 5)
      return;
    pixel.isUsed() = true;
    coalescedpixellist.push_back(pixel);
    /** check for left neighbour */
    if (direction != Direction::east)
    {
      if (pixel.x() != 0)
      {
        pair<uint16_t,uint16_t> left(make_pair(pixel.x()-1,pixel.y()));
        PixelDetector::pixelList_t::iterator neighbourPixelIt = (find_if(pixellist.begin(),
                                                                         pixellist.end(),
                                                                         isNeighbour(left)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::west, pixellist, coalescedpixellist);
      }
    }
    /** check for right neighbour */
    if (direction != Direction::west)
    {
      if (pixel.x() < 1023)
      {
        pair<uint16_t,uint16_t> right(make_pair(pixel.x()+1,pixel.y()));
        PixelDetector::pixelList_t::iterator neighbourPixelIt(find_if(pixellist.begin(),
                                                                      pixellist.end(),
                                                                      isNeighbour(right)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::east, pixellist, coalescedpixellist);
      }
    }
    /** check for top neighbour */
    if (direction != Direction::south)
    {
      if (pixel.y() < 1023)
      {
        pair<uint16_t,uint16_t> top(make_pair(pixel.x(),pixel.y()+1));
        PixelDetector::pixelList_t::iterator neighbourPixelIt(find_if(pixellist.begin(),
                                                                      pixellist.end(),
                                                                      isNeighbour(top)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::north, pixellist, coalescedpixellist);
      }
    }
    /** check for bottom neighbour */
    if (direction != Direction::north)
    {
      if (pixel.y() != 0)
      {
        pair<uint16_t,uint16_t> bottom(make_pair(pixel.x(),pixel.y()-1));
        PixelDetector::pixelList_t::iterator neighbourPixelIt(find_if(pixellist.begin(),
                                                                      pixellist.end(),
                                                                      isNeighbour(bottom)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::south, pixellist, coalescedpixellist);
      }
    }
  }

  /** coalesce one pixel from the list
   *
   * @author Lutz Foucar
   */
  Pixel coalesce(const PixelDetector::pixelList_t &pixellist)
  {
    Pixel pixel(pixellist.front());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin()+1);
    for (; it != pixellist.end(); ++it)
      pixel.z() += it->z();
    return pixel;
  }
}

SimpleCoalesce::SimpleCoalesce()
{}

void SimpleCoalesce::loadSettings()
{
}

PixelDetector::pixelList_t& SimpleCoalesce::operator() (PixelDetector::pixelList_t &pixellist,
                                                        PixelDetector::pixelList_t &coalescedpixels)
{
  PixelDetector::pixelList_t::iterator pixel(pixellist.begin());
  for(; pixel != pixellist.end();++ pixel)
  {
    if (!pixel->isUsed())
    {
      PixelDetector::pixelList_t coalescedpixellist;
      findNeighbours(0,*pixel, Direction::origin, pixellist, coalescedpixellist);
      Pixel coalescedpixel(coalesce(coalescedpixellist));
      coalescedpixels.push_back(coalescedpixel);
    }
  }
  return (coalescedpixels);
}

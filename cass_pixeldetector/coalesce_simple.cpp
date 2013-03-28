// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalesce_simple.cpp contains class that does the pixel coalescing in a
 *                           simple way.
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <limits>
#include <cmath>

#include "coalesce_simple.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{
namespace Direction
{
/** enum for easier code */
enum direction{origin, north, east, south, west};
}

/** check if pixel is neighbour
 *
 * predicate struct for find if. Will return true when a pixel is a neighbour.
 * It will check whether the position of the pixel is identical to the
 * coordinate that was given to this predicate in the constructor. The pixel
 * also is not allowed to be used before as a neighbour of another pixel.
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
    return (!pixel.used &&
            pixel.x == _neighbourCoordinate.first &&
            pixel.y == _neighbourCoordinate.second);
  }
};

/** find all neighbours of a pixel
 *
 * This function is meant to be called recursively only to a depth of 5. First
 * it will add the pixel that is passed to this function to the split pixel
 * list and is marked as used. Then it will try to find another pixel in the
 * pixellist of the  frame, that is a neighbour of that pixel. It will search
 * whether there is  a pixel above, underneath, left or right from the pixel
 * and call this function for this pixel recursively and increases the depth
 * by one. As parameter it will also tell the callee where the orignal pixel
 * was, so that when checking for neightbours we can omit the directions we
 * came from.
 *
 * This idea was inspired by Tom White.
 *
 * @param depth The recursive depth of calling this function
 * @param maxDepth The maximum allowed recursive depth of calling this function
 * @param pixel The pixel whos neighbours we are searching for
 * @param direction The direction that we came from
 * @param frame Reference to the frame containing the frame data an info about
 *              the coulumns and rows of the frame.
 * @param pixels the list of pixels that should be coalesced
 * @param splitpixelslist Reference to the list of pixels that have neighbours
 *                        The event (hit) that all these pixels belong to has
 *                        been split among the pixels in this list.
 *
 * @author Lutz Foucar
 */
void findNeighbours(uint16_t depth,
                    const uint16_t maxDepth,
                    Pixel& pixel,
                    Direction::direction direction,
                    const Frame &frame,
                    CoalescingBase::pixels_t &pixels,
                    CoalescingBase::pixels_t &splitpixelslist)
{
  typedef AdvancedDetector::pixels_t pixels_t;
  typedef pair<uint16_t,uint16_t> position_t;

  if (depth > maxDepth)
    return;
  pixel.used = true;
  splitpixelslist.push_back(pixel);

  /** check for neighbour to the west */
  if (direction != Direction::east)
  {
    if (pixel.x != 0)
    {
      position_t left(make_pair(pixel.x-1,pixel.y));
      CoalescingBase::pixels_t::iterator neighbourPixelIt
          (find_if(pixels.begin(), pixels.end(), isNeighbour(left)));
      if (neighbourPixelIt != pixels.end())
        findNeighbours(depth+1,maxDepth,*neighbourPixelIt, Direction::west, frame, pixels, splitpixelslist);
    }
  }
  /** check for neighbour to the east */
  if (direction != Direction::west)
  {
    if (pixel.x < frame.columns-1)
    {
      position_t right(make_pair(pixel.x+1,pixel.y));
      CoalescingBase::pixels_t::iterator neighbourPixelIt
          (find_if(pixels.begin(), pixels.end(), isNeighbour(right)));
      if (neighbourPixelIt != pixels.end())
        findNeighbours(depth+1,maxDepth,*neighbourPixelIt, Direction::east, frame, pixels, splitpixelslist);
    }
  }
  /** check for neighbour to the north */
  if (direction != Direction::south)
  {
    if (pixel.y < frame.rows-1)
    {
      position_t top(make_pair(pixel.x,pixel.y+1));
      CoalescingBase::pixels_t::iterator neighbourPixelIt
          (find_if(pixels.begin(), pixels.end(), isNeighbour(top)));
      if (neighbourPixelIt != pixels.end())
        findNeighbours(depth+1,maxDepth,*neighbourPixelIt, Direction::north, frame, pixels, splitpixelslist);
    }
  }
  /** check for neighbour to the south*/
  if (direction != Direction::north)
  {
    if (pixel.y != 0)
    {
      position_t bottom(make_pair(pixel.x,pixel.y-1));
      CoalescingBase::pixels_t::iterator neighbourPixelIt
          (find_if(pixels.begin(), pixels.end(), isNeighbour(bottom)));
      if (neighbourPixelIt != pixels.end())
        findNeighbours(depth+1,maxDepth,*neighbourPixelIt, Direction::south, frame, pixels, splitpixelslist);
    }
  }
}

/** coalesce the pixels
 *
 * Coalesce the pixels in the pixel list to an hit on the detector. The value
 * (Z) of the pixels will be added. The position of the hit is defined by
 * the center of mass of the pixel group. Also remember how many pixels
 * contributed to the hit.
 *
 * @return the hit, which is the coalesced pixels
 * @param splitpixelslist The list of pixels that belong to one hit on the
 *                        detector.
 *
 * @author Lutz Foucar
 */
Hit coalesce(const CoalescingBase::pixels_t &splitpixelslist)
{

  Hit hit;
  hit.x = splitpixelslist.front().x;
  hit.y = splitpixelslist.front().y;
  hit.z = splitpixelslist.front().z;
  float weightX(hit.x*hit.z);
  float weightY(hit.y*hit.z);
  CoalescingBase::pixels_t::const_iterator pixel(splitpixelslist.begin()+1);
  for (; pixel != splitpixelslist.end(); ++pixel)
  {
    weightX += (pixel->z*pixel->x);
    weightY += (pixel->z*pixel->y);
    hit.z += pixel->z;
  }
  hit.x = weightX / hit.z;
  hit.y = weightY / hit.z;
  hit.nbrPixels = splitpixelslist.size();
  return hit;
}

/** check whether list of pixel should be coalesced
 *
 * Checks whether the pixels in the list are a massiv ionizing particle (MIP)
 * candidate. This means whether the value (z) of that pixel is above a user
 * set threshold. If this is true then the list should not be coalesced.
 * Then check whether one of the neighbouring pixels in the raw frame is 0.
 * This means that it has been marked as bad by either the darkframe
 * calibration or by the user.
 *
 * @return true when the pixels in the list should be coalesced
 * @param splitpixelslist The list of pixels that belong to one hit on the
 *                        detector.
 * @param mipThreshold The threshold above which a pixel is identified as MIP
 *                     signature
 * @param frame The frame of this detector
 *
 * @author Lutz Foucar
 */
bool shouldCoalescePixel(const CoalescingBase::pixels_t &splitpixelslist,
                         const float mipThreshold,
                         const Frame &frame)
{
  CoalescingBase::pixels_t::const_iterator pixel(splitpixelslist.begin());
  const size_t framewidth(frame.columns);
  const size_t frameheight(frame.rows);
  const frame_t &data(frame.data);
  for (; pixel != splitpixelslist.end(); ++pixel)
  {
    if (pixel->z > mipThreshold)
    {
      //        cout <<" mpiThreshold reached: " <<pixel->z()<<endl;
      return false;
    }
    const size_t idx(pixel->y*framewidth + pixel->x);
    if (pixel->y != 0)
    {
      if (abs(data[idx-framewidth]) < sqrt(numeric_limits<pixel_t>::epsilon()))  //lower middle
      {
        //          cout << "lower middle is " << frame[idx-framewidth]<<endl;
        return false;
      }
      if (pixel->x != 0)
        if (abs(data[idx-framewidth-1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //lower left
        {
          //            cout << "lower left is " << frame[idx-framewidth-1]<<endl;
          return false;
        }
      if (pixel->x < framewidth-1)
        if (abs(data[idx-framewidth+1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //lower right
        {
          //            cout << "lower right is " << frame[idx-framewidth+1]<<endl;
          return false;
        }
    }
    if (pixel->x != 0)
      if (abs(data[idx-1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //left
      {
        //          cout << "left is " << frame[idx-1]<<endl;
        return false;
      }
    if (pixel->x < framewidth-1)
      if (abs(data[idx+1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //right
      {
        //          cout << "right is " << frame[idx+1]<<endl;
        return false;
      }
    if (pixel->y < frameheight-1)
    {
      if (abs(data[idx+framewidth]) < sqrt(numeric_limits<pixel_t>::epsilon())) //upper middle
      {
        //          cout << "upper middle is " << frame[idx+framewidth]<<endl;
        return false;
      }
      if (pixel->x != 0)
        if (abs(data[idx+framewidth-1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //upper left
        {
          //            cout << "upper left is " << frame[idx+framewidth-1]<<endl;
          return false;
        }
      if (pixel->x < framewidth-1)
        if (abs(data[idx+framewidth+1]) < sqrt(numeric_limits<pixel_t>::epsilon())) //upper right
        {
          //            cout << "uppper right is " << frame[idx+framewidth+1]<<endl;
          return false;
        }
    }
  }
  return true;
}

}//end namespace pixeldetector
}//end namespace cass

SimpleCoalesce::SimpleCoalesce()
{}

void SimpleCoalesce::loadSettings(CASSSettings &s)
{
  s.beginGroup("SimpleCoalescing");
  _maxPixelListSize = s.value("MaxPixelListSize",10000).toUInt();
  _maxRecursionDepth = s.value("MaxRecursionDepth",7).toUInt();
  _mipThreshold = s.value("MipThreshold",1e6).toFloat();
  _notCheckCoalesce = s.value("ShouldNotCheckCoalsescing",false).toBool();
  s.endGroup();
}

SimpleCoalesce::hits_t& SimpleCoalesce::operator() (const Frame &frame,
                                                    pixels_t &pixels,
                                                    hits_t &hits)
{
  pixels_t::iterator pixel(pixels.begin());
  if (pixels.size() > _maxPixelListSize)
  {
//    cout << pixellist.size() <<" "<< hits.size()<<endl;
    return hits;
  }
  for(; pixel != pixels.end();++ pixel)
  {
    if (!pixel->used)
    {
      pixels_t splitHit_pixels;
      findNeighbours(0,_maxRecursionDepth,*pixel, Direction::origin, frame, pixels, splitHit_pixels);
      if (_notCheckCoalesce || shouldCoalescePixel(splitHit_pixels,_mipThreshold,frame))
      {
        Hit hit(coalesce(splitHit_pixels));
        hits.push_back(hit);
      }
    }
  }
//  cout <<pixellist.size()<<" "<< hits.size()<<endl;
  return hits;
}

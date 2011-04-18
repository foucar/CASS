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
#include "pixel_detector_container.h"

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
      return (!pixel.isUsed() &&
              pixel.x() == _neighbourCoordinate.first &&
              pixel.y() == _neighbourCoordinate.second);
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
   * @param pixel The pixel whos neighbours we are searching for
   * @param direction The direction that we came from
   * @param det Reference to the detector container containing the frame and
   *            pixellist that we are now trying to find the split pixels in.
   * @param splitpixellist Reference to the list of pixels that have neighbours
   *                       The event that this all these pixels belong to has
   *                       been split among the pxixels in this list.
   *
   * @author Lutz Foucar
   */
  void findNeighbours(uint16_t depth,
                      Pixel& pixel,
                      Direction::direction direction,
                      PixelDetectorContainer &det,
                      PixelDetector::pixelList_t &splitpixelslist)
  {
    typedef PixelDetector::pixelList_t pixelslist_t;
    typedef pair<uint16_t,uint16_t> position_t;

    if (depth > 5)
      return;
    pixel.isUsed() = true;
    splitpixelslist.push_back(pixel);

    pixelslist_t &pixellist(det.pixellist());
    /** check for neighbour to the west */
    if (direction != Direction::east)
    {
      if (pixel.x() != 0)
      {
        position_t left(make_pair(pixel.x()-1,pixel.y()));
        pixelslist_t::iterator neighbourPixelIt
            (find_if(pixellist.begin(), pixellist.end(), isNeighbour(left)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::west, det, splitpixelslist);
      }
    }
    /** check for neighbour to the east */
    if (direction != Direction::west)
    {
      if (pixel.x() < det.pixelDetector().columns()-1)
      {
        position_t right(make_pair(pixel.x()+1,pixel.y()));
        pixelslist_t::iterator neighbourPixelIt
            (find_if(pixellist.begin(), pixellist.end(), isNeighbour(right)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::east, det, splitpixelslist);
      }
    }
    /** check for neighbour to the north */
    if (direction != Direction::south)
    {
      if (pixel.y() < det.pixelDetector().rows()-1)
      {
        position_t top(make_pair(pixel.x(),pixel.y()+1));
        pixelslist_t::iterator neighbourPixelIt
            (find_if(pixellist.begin(), pixellist.end(), isNeighbour(top)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::north, det, splitpixelslist);
      }
    }
    /** check for neighbour to the south*/
    if (direction != Direction::north)
    {
      if (pixel.y() != 0)
      {
        position_t bottom(make_pair(pixel.x(),pixel.y()-1));
        pixelslist_t::iterator neighbourPixelIt
            (find_if(pixellist.begin(), pixellist.end(), isNeighbour(bottom)));
        if (neighbourPixelIt != pixellist.end())
          findNeighbours(depth+1,*neighbourPixelIt, Direction::south, det, splitpixelslist);
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
  PixelDetectorHit coalesce(const PixelDetector::pixelList_t &splitpixelslist)
  {
    PixelDetectorHit hit;
    hit.x() = splitpixelslist.front().x();
    hit.y() = splitpixelslist.front().y();
    hit.z() = splitpixelslist.front().z();
    float weightX(hit.x()*hit.z());
    float weightY(hit.y()*hit.z());
    PixelDetector::pixelList_t::const_iterator pixel(splitpixelslist.begin()+1);
    for (; pixel != splitpixelslist.end(); ++pixel)
    {
      weightX += (pixel->z()*pixel->x());
      weightY += (pixel->z()*pixel->y());
      hit.z() += pixel->z();
    }
    hit.x() = weightX / hit.z();
    hit.y() = weightY / hit.z();
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
   * @param det The detector container that contains the information about this
   *            detector.
   *
   * @author Lutz Foucar
   */
  bool shouldCoalescePixel(const PixelDetector::pixelList_t &splitpixelslist,
                           PixelDetectorContainer &det)
  {
    PixelDetector::pixelList_t::const_iterator pixel(splitpixelslist.begin());
    float mipThreshold(det.mipThreshold());
    for (; pixel != splitpixelslist.end(); ++pixel)
    {
      if (pixel->z() > mipThreshold)
        return false;
      const size_t framewidth(det.pixelDetector().columns());
      const size_t frameheight(det.pixelDetector().rows());
      size_t idx(pixel->y()*framewidth + pixel->x());
      const PixelDetector::frame_t &frame(det.pixelDetector().frame());
      if (pixel->y() != 0)
      {
        if (frame[idx-framewidth] > sqrt(numeric_limits<pixel_t>::epsilon()))  //lower middle
          return false;
        if (pixel->x() != 0)
          if (frame[idx-framewidth-1] > sqrt(numeric_limits<pixel_t>::epsilon())) //lower left
            return false;
        if (pixel->x() < framewidth-1)
          if (frame[idx-framewidth+1] > sqrt(numeric_limits<pixel_t>::epsilon())) //lower right
            return false;
      }
      if (pixel->x() != 0)
        if (frame[idx-1] > sqrt(numeric_limits<pixel_t>::epsilon())) //left
          return false;
      if (pixel->x() < framewidth-1)
        if (frame[idx+1] > sqrt(numeric_limits<pixel_t>::epsilon())) //right
          return false;
      if (pixel->y() < frameheight-1)
      {
        if (frame[idx+framewidth] > sqrt(numeric_limits<pixel_t>::epsilon())) //upper middle
          return false;
        if (pixel->x() != 0)
          if (frame[idx+framewidth-1] > sqrt(numeric_limits<pixel_t>::epsilon())) //upper left
            return false;
        if (pixel->x() < framewidth-1)
          if (frame[idx+framewidth+1] > sqrt(numeric_limits<pixel_t>::epsilon())) //upper right
            return false;
      }
    }
    return true;
  }
}

SimpleCoalesce::SimpleCoalesce()
{}

void SimpleCoalesce::loadSettings(CASSSettings &/*s*/)
{
}

SimpleCoalesce::hitlist_t& SimpleCoalesce::operator() (PixelDetectorContainer &det,
                                                       SimpleCoalesce::hitlist_t &hits)
{
  PixelDetector::pixelList_t &pixellist(det.pixellist());
  PixelDetector::pixelList_t::iterator pixel(pixellist.begin());
  for(; pixel != pixellist.end();++ pixel)
  {
    if (!pixel->isUsed())
    {
      PixelDetector::pixelList_t splitpixellist;
      findNeighbours(0,*pixel, Direction::origin, det, splitpixellist);
      if (shouldCoalescePixel(splitpixellist,det))
      {
        PixelDetectorHit hit(coalesce(splitpixellist));
        hits.push_back(hit);
      }
    }
  }
  return hits;
}

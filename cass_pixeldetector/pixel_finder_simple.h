// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_finder_simple.h contains pixel finder that works like Per Johnsons
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELFINDER_SIMPLE_H_
#define _PIXELFINDER_SIMPLE_H_

#include "pixel_finder_base.h"

namespace cass
{
namespace pixeldetector
{
class PixelFinderSimple : public PixelFinderBase
{
public:
  /** constructor */
  PixelFinderSimple();

  /** find the pixels
   *
   * take the input frame and search it for pixels which are put into the
   * list of pixels.
   *
   * @return reference to the coalesced pixel list
   * @param frame the frame containing the pixels of interest
   * @param pixels the list of pixels that should be found
   */
  pixels_t& operator() (const Frame &frame, pixels_t &pixels);

  /** load the settings of this
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

};
}//end namespace pixeldetector
}//end namespace cass

#endif

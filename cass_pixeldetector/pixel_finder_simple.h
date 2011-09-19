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
/** simple algorithm to find pixels of interest
 *
 * the algorithm is taken from Per Johnsons code and is just put into a functor.
 *
 * @cassttng PixelDetectors/\%name\%/SimpleFinder/{Threshold}\n
 *           The threshold above which the pixels have to be. Default is 0.
 *
 * @author Per Johnson
 * @author Lutz Foucar
 */
class PixelFinderSimple : public PixelFinderBase
{
public:
  /** constructor */
  PixelFinderSimple();

  /** find the pixels
   *
   * take the input frame and search it for pixels which are put into the
   * list of pixels.\n
   * Go through the whole frame and check wether a pixel is above the
   * threshold. If so then check whether the sourrounding pixels all have
   * a smaller value. If this is the case and the pixel is not at the edge of
   * the frame add the pixel to the list of pixels
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

private:
  /** the threshold above which the pixels have to be */
  pixel_t _threshold;
};
}//end namespace pixeldetector
}//end namespace cass

#endif

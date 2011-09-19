// Copyright (C) 2011 Lutz Foucar

/**
 * @file above_noise_finder.h contains hll like finder for pixels
 *
 * @author Lutz Foucar
 */

#ifndef _ABOVENOISEFINDER_H_
#define _ABOVENOISEFINDER_H_

#include "pixel_finder_base.h"

namespace cass
{
namespace pixeldetector
{
class AboveNoiseFinder : public PixelFinderBase
{
public:
  /** constructor */
  AboveNoiseFinder();

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

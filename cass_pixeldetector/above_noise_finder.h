// Copyright (C) 2011 Lutz Foucar

/**
 * @file above_noise_finder.h contains hll like finder for pixels
 *
 * @author Lutz Foucar
 */

#ifndef _ABOVENOISEFINDER_H_
#define _ABOVENOISEFINDER_H_

#include "pixel_finder_base.h"
#include "common_data.h"

namespace cass
{
namespace pixeldetector
{
/** will find pixels by comparing them to a "noise" map
 *
 * This HLL like finding of pixels will compare the pixel values to a "noise"
 * Map (see cass::pixeldetector::CommonData::noiseMap).
 *
 * @author Lutz Foucar
 */
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
   * Will go through the whole frame and compare each pixel whether its value
   * is bigger than _multiplier * noise taken from the noiseMap. If this is
   * the case it will be added to the list of pixels.
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
  /** the global data container for the detector */
  CommonData::shared_pointer _commondata;

  /** how many times the value of the pixel needs to be higher than the standart deviation */
  float _multiplier;
};
}//end namespace pixeldetector
}//end namespace cass

#endif

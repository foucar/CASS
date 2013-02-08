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
 * @note the multiplication should be done here since if we do it on the global
 *       noise map it will be the same for all detectors @todo is this right?
 *
 * @cassttng PixelDetectors/\%name\%/AboveNoiseFinder/{Multiplier}\n
 *           Value multiplied to the noise value before comparing whether the
 *           pixel is above the noise. Default is 4.
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



/** will find pixels by comparing them to a "noise" map and descrimnate them
 *  against local background
 *
 * This HLL like finding of pixels will compare the pixel values to a "noise"
 * Map (see cass::pixeldetector::CommonData::noiseMap). After a pixel candidate
 * has been found one checks its local background in a box. Is the pixelvalue
 * after substracting this local background still higher than xxx it will be added to the
 * list of pixels.
 *
 * @note the multiplication should be done here since if we do it on the global
 *       noise map it will be the same for all detectors @todo is this right?
 *
 * @cassttng PixelDetectors/\%name\%/AboveNoiseFinder/{Multiplier}\n
 *           Value multiplied to the noise value before comparing whether the
 *           pixel is above the noise. Default is 4.
 *
 * @author Lutz Foucar
 */
class AdvancedAboveNoiseFinder : public PixelFinderBase
{
public:
  /** constructor */
  AdvancedAboveNoiseFinder();

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

  /** the threshold */
  float _threshold;

  /** the size of the box used for the median filter */
  std::pair<uint16_t,uint16_t> _boxSize;
};
}//end namespace pixeldetector
}//end namespace cass

#endif

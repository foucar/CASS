// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_finder_base.h contains base class for all pixel finders.
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELFINDERBASE_H_
#define _PIXELFINDERBASE_H_

#include <tr1/memory>
#include <vector>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;

/** base class for pixel finders
 *
 * a pixel finder should identifiy the pixels of interest within a frame
 *
 * @author Lutz Foucar
 */
class PixelFinderBase
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<PixelFinderBase> shared_pointer;

  /** the list of pixels */
  typedef std::vector<Pixel> pixels_t;

  /** virtual destructor */
  virtual ~PixelFinderBase() {}

  /** create an instance of the requested functor
   *
   * @return a shared pointer to the requested type
   * @param type the reqested type
   */
  static shared_pointer instance(const std::string &type);

  /** find the pixels
   *
   * take the input frame and search it for pixels which are put into the
   * list of pixels.
   *
   * @return reference to the coalesced pixel list
   * @param frame the frame containing the pixels of interest
   * @param pixels the list of pixels that should be found
   */
  virtual pixels_t& operator() (const Frame &frame, pixels_t &pixels)=0;

  /** load the settings of this
   *
   * @param s the CASSSettings object to read the information from
   */
  virtual void loadSettings(CASSSettings &s)=0;
};

} //end namespace pixeldetector
} //end namespace cass
#endif

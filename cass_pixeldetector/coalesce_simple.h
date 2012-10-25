// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalesce_simple.h contains class that does the pixel coalescing in a
 *                         simple way.
 *
 * @author Lutz Foucar
 */

#ifndef _COALESCESIMPLE_H_
#define _COALESCESIMPLE_H_

#include "coalescing_base.h"

namespace cass
{
namespace pixeldetector
{

/** simple coalescing of pixels
 *
 * take the input pixel list and coalesce it to hits on the detector.
 *
 * @cassttng PixelDetectors/\%name\%/SimpleCoalescing/{MaxPixelListSize}\n
 *           Maximum size of the incomming pixel list that will be still
 *           treated by this functor. Default is 10000
 * @cassttng PixelDetectors/\%name\%/SimpleCoalescing/{MipThreshold}\n
 *           The threhold in ADU above which a pixel is regarded as part of
 *           a Ionizing Particle. Default is 1e6
 * @cassttng PixelDetectors/\%name\%/SimpleCoalescing/{ShouldNotCheckCoalsescing}\n
 *           Once a sublist of pixels that should be coalesced is found,
 *           usually the function checks whether it should be coalesced based
 *           on whether non of the sourrounding pixels is 0. This is bad when
 *           using this with detectors other than pnCCD. This flag when true
 *           will disable this check. Default is 'false'
 * @cassttng PixelDetectors/\%name\%/SimpleCoalescing/{MaxRecursionDepth}\n
 *           The maximum recursion depth whith which the recursive search for
 *           neighbouring pixels will be done. For details see
 *           cass::findNeighbours(). Default is 7
 *
 * @author Lutz Foucar
 */
class SimpleCoalesce : public CoalescingBase
{
public:
  /** constructor */
  SimpleCoalesce();

  /** coalesce the pixels
   *
   * use a simple recursive algorithm to find pixels that are neighbours. See
   * cass::findNeighbours(). After a check whether the pixels should be
   * coalesced, see cass::shouldCoalescePixel(), coalesce the pixels to hits,
   * see cass::coalesce()
   *
   * @return reference to the coalesced pixel list
   * @param frame the frame containing the pixels of interest
   * @param pixels the list of pixels that should be coalesced
   * @param hits The list where the pixels that were coalesced to hits go in
   */
  hits_t& operator() (const Frame &frame, pixels_t &pixels, hits_t &hits);

  /** load the settings of this
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** the maximmum size of the incomming pixel list that we will still work on */
  size_t _maxPixelListSize;

  /** how many times is the recursion be allowe to call itselve */
  uint16_t _maxRecursionDepth;

  /** the threshold above which a pixel is identified as MIP signature */
  float _mipThreshold;

  /** flag to show whether the pixels should be coalesced */
  bool _notCheckCoalesce;
};

} //end namespace pixeldetector
} //end namespace cass
#endif

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
  /** simple coalescing of pixels
   *
   * use a simple algorithm that coalesces the pixels of a PixelDetector
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
     * take the input pixel list and coalesce it
     *
     * @return reference to the coalesced pixel list
     * @param det the detector that contains the relevant pixellist
     * @param coalescedpixles the list where the coalesced pixels go in
     */
    PixelDetector::pixelList_t& operator() (PixelDetectorContainer &det,
                                            PixelDetector::pixelList_t &coalescedpixels);

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    void loadSettings(CASSSettings &s);
  };
}
#endif

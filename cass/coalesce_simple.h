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
   * take the input pixel list and coalesce it to hits on the detector.
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
     * @return reference to the list of hits
     * @param det the detector that contains the relevant pixellist
     * @param hits The list where the pixels that were coalesced to hits go in
     */
    hitlist_t& operator() (PixelDetectorContainer &det,
                           hitlist_t &hits);

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    void loadSettings(CASSSettings &s);
  };
}
#endif

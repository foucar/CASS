// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.h file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#ifndef _COALESCINGBASE_H_
#define _COALESCINGBASE_H_

#include <tr1/memory>

#include "pixel_detector_container.h"

namespace cass
{
  //forward declaration//
  class CASSSettings;
  class PixelDetectorContainer;

  /** base class for all coalescing functors
   *
   * @author Lutz Foucar
   */
  class CoalescingBase
  {
  public:
    /** typedef the shared pointer of this */
    typedef std::tr1::shared_ptr<CoalescingBase> shared_pointer;

    /** redefine the coalesced pixels list for shorter code */
    typedef PixelDetectorContainer::hitlist_t hitlist_t;

    /** virtual destructor */
    virtual ~CoalescingBase() {}

    /** create an instance of the requested functor
     *
     * @return a shared pointer to the requested type
     * @param type the reqested type
     */
    static shared_pointer instance(const std::string &type);

    /** coalesce the pixels
     *
     * take the input pixel list and coalesce it to hits on the detector.
     *
     * @return reference to the coalesced pixel list
     * @param det the detector that contains the relevant pixellist
     * @param hits The list where the pixels that were coalesced to hits go in
     */
    virtual hitlist_t& operator() (PixelDetectorContainer &det,
                                   hitlist_t &hits)=0;

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    virtual void loadSettings(CASSSettings &s)=0;
  };
}
#endif

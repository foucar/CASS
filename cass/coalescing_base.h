// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.h file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */
#ifndef _COALESCINGBASE_H_
#define _COALESCINGBASE_H_

#include <tr1/memory>

#include "pixel_detector.h"

namespace cass
{
  /** base class for all coalescing functors
   *
   * @author Lutz Foucar
   */
  class CoalescingBase
  {
  public:
    /** typedef the shared pointer of this */
    typedef std::tr1::shared_ptr<CoalescingBase> shared_pointer;

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
     * take the input pixel list and coalesce it
     *
     * @param pixellist the list of non coalesced pixels
     * @param coalescedpixles the list where the coalesced pixels go in
     */
    virtual PixelDetector::pixelList_t operator() (const PixelDetector::pixelList_t &pixellist,
                                                   PixelDetector::pixelList_t &coalescedpixels)=0;

    /** load the settings of the functor */
    virtual void loadSettings()=0;
  };
}
#endif

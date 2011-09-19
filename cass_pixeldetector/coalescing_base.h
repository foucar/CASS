// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.h file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#ifndef _COALESCINGBASE_H_
#define _COALESCINGBASE_H_

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

/** base class for all coalescing functors
 *
 * coalscing function should coalesce pixels found in an analysis to form hits
 * on a pixel detector.
 *
 * @author Lutz Foucar
 */
class CoalescingBase
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<CoalescingBase> shared_pointer;

  /** define the list of coalesced pixels */
  typedef std::vector<Hit> hits_t;

  typedef std::vector<Pixel> pixels_t;

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
   * @param frame the frame containing the pixels of interest
   * @param pixels the list of pixels that should be coalesced
   * @param hits The list where the pixels that were coalesced to hits go in
   */
  virtual hits_t& operator() (const Frame &frame, pixels_t pixels, hits_t &hits)=0;

  /** load the settings of this
   *
   * @param s the CASSSettings object to read the information from
   */
  virtual void loadSettings(CASSSettings &s)=0;
};

} //end namespace pixeldetector
} //end namespace cass
#endif

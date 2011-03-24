// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_container.h file container for pixeldetectors
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELDETECTORCONTAINER_H_
#define _PIXELDETECTORCONTAINER_H_

#include <tr1/memory>

#include "pixel_detector.h"

namespace cass
{
  //forward declaration
  class CoalescingBase;
  class CASSSettings;

  /** PixelDetector Manager
   *
   * This class contains a pointer to a pixel detector and extents the a regular
   * pixel detector by a coalesced pixel list. The coalesced pixel list will
   * be created with the help of the _coalesce functor
   *
   * @author Lutz Foucar
   */
  class PixelDetectorContainer
  {
  public:
    /** associate the container with a Pixel Detector
     *
     * @param in Pointer to the CASSEvent that contains the PixelDetector that
     *           this container is responsible for.
     */
    void associate(const CASSEvent &in);

    /** retrieve reference to the managed pixeldetector */
    PixelDetector & pixelDetector() {return *_pixeldetector;}

    /** retrieve the coalesced pixel list */
    const PixelDetector::pixelList_t& coalescedPixels();

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    void loadSettings(CASSSettings &s);

  private:
    /** pointer to the pixel detector we manage */
    PixelDetector *_pixeldetector;

    /** coaleced pixellist
     *
     * the coaleced pixellist contains the pixel after the split pixels have
     * been reunited again.
     */
    PixelDetector::pixelList_t  _coalescedpixels;

    /** flag whehter coaleced pixel list has been created already */
    bool _coalescedCreated;

    /** functor that will do the coalescing */
    std::tr1::shared_ptr<CoalescingBase> _coalesce;
  };
}
#endif

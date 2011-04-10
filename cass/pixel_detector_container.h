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
  class CASSEvent;

  /** PixelDetector Wrapper
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
    /** constructor
     *
     * @param name the name of this container
     */
    PixelDetectorContainer(const std::string &name)
      : _coalescedCreated(false),
        _name(name)
    {}

    /** associate the container with a Pixel Detector
     *
     * @param evt Pointer to the CASSEvent that contains the PixelDetector that
     *            this container is responsible for.
     */
    void associate(const CASSEvent &evt);

    /** retrieve reference to the managed pixeldetector */
    const PixelDetector &pixelDetector() {return *_pixeldetector;}

    /** retrieve the coalesced pixel list */
    const PixelDetector::pixelList_t& coalescedPixels();

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    void loadSettings(CASSSettings &s);

    /** retrieve the pixellist */
    PixelDetector::pixelList_t& pixellist() {return _pixelslist;}

  private:
    /** pointer to the pixel detector we manage */
    const PixelDetector *_pixeldetector;

    /** pixellist
     *
     * this is a copy of cass::PixelDetector::_pixelslist
     */
    PixelDetector::pixelList_t _pixelslist;

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

    /** the name of this container */
    std::string _name;

    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    int32_t _device;
  };
}
#endif

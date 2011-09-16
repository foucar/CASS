// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_new.h advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELDETECTORNEW_H_
#define _PIXELDETECTORNEW_H_

#include <tr1/memory>

namespace cass
{
  //forward declaration
  class CoalescingBase;
  class CASSSettings;
  class CASSEvent;


  /** An Advanced Pixel Detector
   *
   * This class describes a pixel detector which has all the opertors to analyse
   * and extract the additional information internally.
   *
   * @cassttng PixelDetectors/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PixelDetectors/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PixelDetectors/\%name\%/{CoalescingFunctionType}\n
   *           Functor to coalesce the pixels into hits. Default is "simple". Options are:
   *           - "simple": simple coalescing with basic checks.
   *                       See cass::SimpleCoalesce.
   *
   * @author Lutz Foucar
   */
  class AdvancedPixelDetector
  {
  public:
    /** define the list of coalesced pixels */
    typedef std::vector<PixelDetectorHit> hitlist_t;
    /** constructor
     *
     * @param name the name of this container
     */
    AdvancedPixelDetector(const std::string &name)
      : _hitListCreated(false),
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
    const hitlist_t& hits();

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

    /** hits on the detector
     *
     * a hit on the detector can be split among several pixels.
     */
    hitlist_t _hits;

    /** flag whehter hit list has been created already */
    bool _hitListCreated;

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

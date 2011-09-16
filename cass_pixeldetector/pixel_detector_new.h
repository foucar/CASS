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

  /** A Hit on a pixel detector.
   *
   * This class defines a hit on a pixel detector.
   *
   * @author Lutz Foucar
   */
  class PixelDetectorHit
  {
  public:
    /** default constructor.*/
    PixelDetectorHit()
      :_x(0),_y(0),_z(0),_nbrPixels(0)
    {}

  public:
    //@{
    /** setter */
    float    &x()         {return _x;}
    float    &y()         {return _y;}
    pixel_t  &z()         {return _z;}
    size_t   &nbrPixels() {return _nbrPixels;}
    //@}
    //@{
    /** getter */
    float     x()const          {return _x;}
    float     y()const          {return _y;}
    pixel_t   z()const          {return _z;}
    size_t    nbrPixels()const  {return _nbrPixels;}
    //@}

  private:
    float    _x;          //!< x coordinate of the pixel
    float    _y;          //!< y coordinate of the pixel
    pixel_t  _z;          //!< the pixel value
    size_t   _nbrPixels;  //!< how many pixels in the frame belong to this hit.
  };

  /** PixelDetector Wrapper
   *
   * This class contains a pointer to a pixel detector and extents the a regular
   * pixel detector by a coalesced pixel list. The coalesced pixel list will
   * be created with the help of the _coalesce functor
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
  class PixelDetectorContainer
  {
  public:
    /** define the list of coalesced pixels */
    typedef std::vector<PixelDetectorHit> hitlist_t;
    /** constructor
     *
     * @param name the name of this container
     */
    PixelDetectorContainer(const std::string &name)
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

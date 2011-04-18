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

  /** Advanced Pixel definition
   *
   * This class defines an advanced pixel in a pixel detector.
   *
   * @author Lutz Foucar
   */
  class AdvancedPixel
  {
  public:
    /** default constructor.*/
    AdvancedPixel()
      :_x(0),_y(0),_z(0),_used(false)
    {}

  public:
    //@{
    /** setter */
    float    &x()       {return _x;}
    float    &y()       {return _y;}
    pixel_t  &z()       {return _z;}
    bool     &isUsed()  {return _used;}
    //@}
    //@{
    /** getter */
    float     x()const      {return _x;}
    float     y()const      {return _y;}
    pixel_t   z()const      {return _z;}
    bool      isUsed()const {return _used;}
    //@}

  private:
    float    _x;    //!< x coordinate of the pixel
    float    _y;    //!< y coordinate of the pixel
    pixel_t  _z;    //!< the pixel value
    bool     _used; //!< flag used in coalescing
  };

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
    /** define the list of coalesced pixels */
    typedef std::vector<AdvancedPixel> coalescedpixelslist_t;
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
    const coalescedpixelslist_t& coalescedPixels();

    /** load the settings of this
     *
     * @param s the CASSSettings object to read the information from
     */
    void loadSettings(CASSSettings &s);

    /** retrieve the pixellist */
    PixelDetector::pixelList_t& pixellist() {return _pixelslist;}

    /** retrieve the MIP threshold */
    float mipThreshold()const   {return _mipThreshold;}

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
    coalescedpixelslist_t _coalescedpixels;

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

    /** the threshold above which a pixel is identified as MIP signature */
    float _mipThreshold;
  };
}
#endif

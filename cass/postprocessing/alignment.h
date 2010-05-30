// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#ifndef _ALGINMENT_POSTPROCESSOR_H_
#define _ALGINMENT_POSTPROCESSOR_H_

#include "backend.h"
#include "postprocessor.h"
#include "cass_event.h"

namespace cass
{
class Histogram0DFloat;

///*! \f$\cos^2\theta\f$ of CCD image
//
//This postprocessor reduces the running average of the CCD image (pp121) to a scalar that represents
//the \f$\cos^2\theta\f$ (degree of alignment)
//
//@author Jochen Küpper
//*/
//class pp131 : public PostprocessorBackend
//{
//public:
//
//    /*! Construct postprocessor for Gaussian height of image */
//    pp131(PostProcessors&, PostProcessors::id_t);
//
//    /** Free _image space */
//    virtual ~pp131();
//
//    /*! Determine Gaussian width of image */
//    virtual void operator()(const CASSEvent&);
//
//    /*! Define postprocessor dependency on pp121 (averaged VMI image) */
//    virtual std::list<PostProcessors::id_t> dependencies();
//
//
//protected:
//
//    Histogram0DFloat *_value;
//};
//
//
//
///*! Gaussian width of CCD image
//
//This reduced the running average of the CCD image (pp121) to a scalar
//that represents the FWHM of a Gaussain fit to the column histogram
//
//@author Jochen Küpper
//*/
//class pp143 : public PostprocessorBackend
//{
//public:
//
//    /*! Construct postprocessor for Gaussian height of image */
//    pp143(PostProcessors&, PostProcessors::id_t);
//
//    /** Free _image space */
//    virtual ~pp143();
//
//    /*! Determine Gaussian width of image */
//    virtual void operator()(const CASSEvent&);
//
//    /*! Define postprocessor dependency on pp121 (averaged VMI image) */
//    virtual std::list<PostProcessors::id_t> dependencies();
//
//
//protected:
//
//    Histogram0DFloat *_value;
//};
//
//
//
///*! Gaussian height of CCD image
//
//This reduced the running average of the CCD image (pp121) to a scalar
//that represents the FWHM of a Gaussain fit to the row histogram
//
//@author Jochen Küpper
//@todo Uncouple 143 and 144 -- I have only done that for demonstration puposes
//*/
//class pp144 : public PostprocessorBackend
//{
//public:
//
//    /*! Construct postprocessor for Gaussian width of image */
//    pp144(PostProcessors&, PostProcessors::id_t);
//
//    /*! Free _image space */
//    virtual ~pp144();
//
//    /*! Determine Gaussian height of image */
//    virtual void operator()(const CASSEvent&);
//
//    /*! Define postprocessor dependency on pp121 (averaged VMI image) */
//    virtual std::list<PostProcessors::id_t> dependencies();
//
//
//protected:
//
//    Histogram0DFloat *_value;
//};




/** \f$\cos^2\theta\f$ of a requested image.
 *
 * This postprocessor reduces the running average of the requested image
 * to a scalar that represents the \f$\cos^2\theta\f$ (degree of alignment).
 *
 * @cassttng PostProcessor/\%name%/{ImageXCenter|ImageYCenter}\n
 *           values for the center of the image. Default is 0,0
 * @cassttng PostProcessor/p\%name%/{SymmetryAngle}\n
 *           value for the symmetry angle. Default is 0.
 * @cassttng PostProcessor/\%name%/{ImageId}\n
 *           The id of the PostProcessor that contains the image to calculate
 *           \f$\cos^2\theta\f$  from. Default is 104.
 * @cassttng PostProcessor/\%name%/{MaxIncludedRadius|MinIncludedRadius}\n
 *           values for the interesting radius range. Default is 0,0
 * @cassttng PostProcessor/\%name%/{DrawInnerOuterRadius}\n
 *           draw the inner and out include radius. default is false
 *
 * @author Per Johnsson
 * @author Lutz Foucar
 */
class pp200 : public PostprocessorBackend
{
public:
  /** Construct postprocessor for Gaussian height of image */
  pp200(PostProcessors&, const PostProcessors::key_t&);

  /** Free _image space */
  virtual ~pp200();

  /** calculate \f$\cos^2\theta\f$ of averaged image */
  virtual void operator()(const CASSEvent&);

  /** Define postprocessor dependency on the requested image*/
  virtual PostProcessors::active_t dependencies();

  /** load the histogram settings from cass.ini*/
  virtual void loadSettings(size_t);

protected:
  std::pair<float, float> _center; //!< Image center
  float _minRadius;                //!< Minimum radius for analysis
  float _maxRadius;                //!< Maximum radius for analysis
  float _symAngle;                 //!< Symmetry angle
  size_t _imageWidth;              //!< Image width
  size_t _nbrRadialPoints;         //!< Number of radial points
  size_t _nbrAngularPoints;        //!< Number of angular points
  bool _drawCircle;               //!< flag to tell whether to draw the inner and outer circle

  /** image that we will calculate the \f$\cos^2\theta\f$ from */
  PostProcessors::key_t   _imagekey;

  /** the cos2theta value */
  Histogram0DFloat *_value;
};

}

#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

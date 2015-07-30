// Copyright (C) 2010 Jochen Kuepper
// Copyright (C) 2010-2013 Lutz Foucar

#ifndef _ALGINMENT_POSTPROCESSOR_H_
#define _ALGINMENT_POSTPROCESSOR_H_

#include "processor.h"
#include "cass_event.h"

namespace cass
{
class Histogram0DFloat;

/** \f$\cos^2\theta\f$ of a requested image.
 *
 * @PPList "200":\f$\cos^2\theta\f$ of a requested image.
 *
 * This postprocessor reduces the running average of the requested image
 * to a scalar that represents the \f$\cos^2\theta\f$ (degree of alignment).
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name%/{HistName}\n
 *           The name of the Processor that contains the image to calculate
 *           \f$\cos^2\theta\f$  from. Default is 104.
 * @cassttng Processor/\%name%/{ImageXCenter|ImageYCenter}\n
 *           values for the center of the image. Default is 0,0
 * @cassttng Processor/p\%name%/{SymmetryAngle}\n
 *           value for the symmetry angle. Default is 0.
 * @cassttng Processor/\%name%/{MaxIncludedRadius|MinIncludedRadius}\n
 *           values for the interesting radius range. Default is 0,0
 *
 * @author Per Johnsson
 * @author Lutz Foucar
 */
class pp200 : public Processor
{
public:
  /** Construct postprocessor for Gaussian height of image */
  pp200(const name_t &);

  /** calculate \f$\cos^2\theta\f$ of averaged image */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the histogram settings from CASS.ini*/
  virtual void loadSettings(size_t);

protected:
  /** center of the image in histogram coordinates */
  std::pair<size_t, size_t> _center;

  /** the range of radia used */
  std::pair<float,float> _radiusRange;

  /** symmetry angle for calculation */
  float _symAngle;

  /** the width of the image */
  size_t _imageWidth;

  /** the number of angular points that we include in the distribution */
  size_t _nbrAngularPoints;

  /** the number of radial, determinded by the _radiusRange */
  size_t _nbrRadialPoints;

  /** pp containing image that we will calculate the \f$\cos^2\theta\f$ from */
  shared_pointer _image;
};




/** angular distribution of a requested image.
 *
 * @PPList "201":angular distribution of a requested image.
 *
 * this postprocessor will iterate through the requested radius set and
 * all angles. Then it transforms the phi,r to kartesian coordinates to find
 * the pixel in the image that the r,phi values corrospond to. It will do a
 * 2D interpolation to be able to weight the content of the pixel correctly.
 * The weighing factor is determined from the distance that the transformed
 * kartesian coordinates have from the neighboring pixels. 0 deg is up.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name%/{HistName}\n
 *           The name of the Processor that contains the image that
 *           the angluar distribution should be retrieved from.
 * @cassttng Processor/\%name%/{ImageXCenter|ImageYCenter}\n
 *           values for the center of the image. Default is 500,500
 * @cassttng Processor/\%name%/{MaxIncludedRadius|MinIncludedRadius}\n
 *           values for the interesting radius range. Default is 10,0
 * @cassttng Processor/\%name%/{NbrAngularPoints}\n
 *           The number of Bins in the resulting histogram
 *
 * @author Per Johnsson
 * @author Marc Vrakking
 * @author Lutz Foucar
 */
class pp201 : public Processor
{
public:
  /** Construct postprocessor for Gaussian height of image */
  pp201(const name_t &);

  /** calculate \f$\cos^2\theta\f$ of averaged image */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the histogram settings from CASS.ini*/
  virtual void loadSettings(size_t);

protected:
  /** center of the image in histogram coordinates */
  std::pair<size_t, size_t> _center;

  /** the rane of radia used */
  std::pair<float,float> _radiusRange;

  /** the number of angular points that we include in the distribution */
  size_t _nbrAngularPoints;

  /** the number of radial, determinded by the _radiusRange */
  size_t _nbrRadialPoints;

  /** pp containing image that we will the angular distribution from */
  shared_pointer _image;
};





/** transform kartesian to poloar coordinates
 *
 * @PPList "202":transform kartesian to poloar coordinates
 *
 * This postprocessor transforms the kartesian coordinates of an image to its
 * polar representation. It transforms the phi, r to kartesian coordinates to
 * find the pixel in the image that the r,phi values corrospond to. It will
 * do a 2D interpolation to be able to weight the content of the pixel
 * correctly. The weighing factor is determined from the distance that the
 * transformed kartesian coordinates have from the neighboring pixels. 0 deg
 * is up.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name%/{HistName}\n
 *           The name of the Processor that contains the image that
 *           the angluar distribution should be retrieved from.
 * @cassttng Processor/\%name%/{ImageXCenter|ImageYCenter}\n
 *           values for the center of the image. Default is 500,500
 * @cassttng Processor/\%name%/{NbrAngularPoints}\n
 *           The number of Bins along the phi axis in the resulting histogram.
 *           Default is 360.
 * @cassttng Processor/\%name%/{NbrRadialPoints}\n
 *           The number of Bins along the r axis in the resulting histogram.
 *           Default is 500.
 *
 * @author Per Johnsson
 * @author Marc Vrakking
 * @author Lutz Foucar
 */
class pp202 : public Processor
{
public:
  /** Construct postprocessor for Gaussian height of image */
  pp202(const name_t &);

  /** calculate \f$\cos^2\theta\f$ of averaged image */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the histogram settings from CASS.ini*/
  virtual void loadSettings(size_t);

protected:
  /** center of the image in user coordinates */
  std::pair<float, float> _userCenter;

  /** center of the image in histogram coordinates */
  std::pair<size_t, size_t> _center;

  /** the maximal radius possible */
  float _maxRadius;

  /** the number of angular points that we include in the distribution */
  size_t _nbrAngularPoints;

  /** the number of radial, determinded by the _radiusRange */
  size_t _nbrRadialPoints;

  /** pp containing image that we will the angular distribution from */
  shared_pointer _image;
};

}//end namespace

#endif

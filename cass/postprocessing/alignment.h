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

  // /*! \f$\cos^2\theta\f$ of CCD image
  //
  //This postprocessor reduces the running average of the CCD image (pp121) to a scalar that represents
  //the \f$\cos^2\theta\f$ (degree of alignment)
  //
  //@author Jochen Küpper
  // */
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
  // /*! Gaussian width of CCD image
  //
  //This reduced the running average of the CCD image (pp121) to a scalar
  //that represents the FWHM of a Gaussain fit to the column histogram
  //
  //@author Jochen Küpper
  // */
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
  // /*! Gaussian height of CCD image
  //
  //This reduced the running average of the CCD image (pp121) to a scalar
  //that represents the FWHM of a Gaussain fit to the row histogram
  //
  //@author Jochen Küpper
  //@todo Uncouple 143 and 144 -- I have only done that for demonstration puposes
  // */
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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name%/{HistName}\n
   *           The name of the PostProcessor that contains the image to calculate
   *           \f$\cos^2\theta\f$  from. Default is 104.
   * @cassttng PostProcessor/\%name%/{ImageXCenter|ImageYCenter}\n
   *           values for the center of the image. Default is 0,0
   * @cassttng PostProcessor/p\%name%/{SymmetryAngle}\n
   *           value for the symmetry angle. Default is 0.
   * @cassttng PostProcessor/\%name%/{MaxIncludedRadius|MinIncludedRadius}\n
   *           values for the interesting radius range. Default is 0,0
   *
   * @author Per Johnsson
   * @author Lutz Foucar
   */
  class pp200 : public PostprocessorBackend
  {
  public:
    /** Construct postprocessor for Gaussian height of image */
    pp200(PostProcessors&, const PostProcessors::key_t&);

    /** calculate \f$\cos^2\theta\f$ of averaged image */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from CASS.ini*/
    virtual void loadSettings(size_t);

    /** adjust the parameters when the incomming histogram has changed */
    virtual void histogramsChanged(const HistogramBackend *in);

  protected:
    /** set up the paramters
     *
     * sets up the parameters that depend on the histogram we depend on
     *
     * @param hist the histogram, whos information we use to set up the parameters
     */
    void setupParameters(const HistogramBackend &hist);

    /** center of the image in user coordinates */
    std::pair<float, float> _userCenter;

    /** center of the image in histogram coordinates */
    std::pair<size_t, size_t> _center;

    /** the range of radia the user requested */
    std::pair<float,float> _radiusRangeUser;

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
    PostprocessorBackend *_image;
  };




  /** angular distribution of a requested image.
   *
   * this postprocessor will iterate through the requested radius set and
   * all angles. Then it transforms the phi,r to kartesian coordinates to find
   * the pixel in the image that the r,phi values corrospond to. It will do a
   * 2D interpolation to be able to weight the content of the pixel correctly.
   * The weighing factor is determined from the distance that the transformed
   * kartesian coordinates have from the neighboring pixels. 0 deg is up.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name%/{HistName}\n
   *           The name of the PostProcessor that contains the image that
   *           the angluar distribution should be retrieved from.
   * @cassttng PostProcessor/\%name%/{ImageXCenter|ImageYCenter}\n
   *           values for the center of the image. Default is 500,500
   * @cassttng PostProcessor/\%name%/{MaxIncludedRadius|MinIncludedRadius}\n
   *           values for the interesting radius range. Default is 10,0
   * @cassttng PostProcessor/\%name%/{NbrAngularPoints}\n
   *           The number of Bins in the resulting histogram
   *
   * @author Per Johnsson
   * @author Marc Vrakking
   * @author Lutz Foucar
   */
  class pp201 : public PostprocessorBackend
  {
  public:
    /** Construct postprocessor for Gaussian height of image */
    pp201(PostProcessors&, const PostProcessors::key_t&);

    /** calculate \f$\cos^2\theta\f$ of averaged image */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from CASS.ini*/
    virtual void loadSettings(size_t);

    /** adjust the parameters when the incomming histogram has changed */
    virtual void histogramsChanged(const HistogramBackend *in);

  protected:
    /** set up the paramters
     *
     * sets up the parameters that depend on the histogram we depend on
     *
     * @param hist the histogram, whos information we use to set up the parameters
     */
    void setupParameters(const HistogramBackend &hist);

    /** center of the image in user coordinates */
    std::pair<float, float> _userCenter;

    /** center of the image in histogram coordinates */
    std::pair<size_t, size_t> _center;

    /** the range of radia the user requested */
    std::pair<float,float> _radiusRangeUser;

    /** the rane of radia used */
    std::pair<float,float> _radiusRange;

    /** the number of angular points that we include in the distribution */
    size_t _nbrAngularPoints;

    /** the number of radial, determinded by the _radiusRange */
    size_t _nbrRadialPoints;

    /** pp containing image that we will the angular distribution from */
    PostprocessorBackend* _image;
  };





  /** transform kartesian to poloar coordinates
   *
   * This postprocessor transforms the kartesian coordinates of an image to its
   * polar representation. It transforms the phi, r to kartesian coordinates to
   * find the pixel in the image that the r,phi values corrospond to. It will
   * do a 2D interpolation to be able to weight the content of the pixel
   * correctly. The weighing factor is determined from the distance that the
   * transformed kartesian coordinates have from the neighboring pixels. 0 deg
   * is up.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name%/{HistName}\n
   *           The name of the PostProcessor that contains the image that
   *           the angluar distribution should be retrieved from.
   * @cassttng PostProcessor/\%name%/{ImageXCenter|ImageYCenter}\n
   *           values for the center of the image. Default is 500,500
   * @cassttng PostProcessor/\%name%/{NbrAngularPoints}\n
   *           The number of Bins along the phi axis in the resulting histogram.
   *           Default is 360.
   * @cassttng PostProcessor/\%name%/{NbrRadialPoints}\n
   *           The number of Bins along the r axis in the resulting histogram.
   *           Default is 500.
   *
   * @author Per Johnsson
   * @author Marc Vrakking
   * @author Lutz Foucar
   */
  class pp202 : public PostprocessorBackend
  {
  public:
    /** Construct postprocessor for Gaussian height of image */
    pp202(PostProcessors&, const PostProcessors::key_t&);

    /** calculate \f$\cos^2\theta\f$ of averaged image */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from CASS.ini*/
    virtual void loadSettings(size_t);

    /** adjust the parameters when the incomming histogram has changed */
    virtual void histogramsChanged(const HistogramBackend *in);

  protected:
    /** set up the paramters
     *
     * sets up the parameters that depend on the histogram we depend on
     *
     * @param hist the histogram, whos information we use to set up the parameters
     */
    void setupParameters(const HistogramBackend &hist);

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
    PostprocessorBackend* _image;
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

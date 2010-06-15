// Copyright (C) 2010 Lutz Foucar

#ifndef _IMAGING_H_
#define _IMAGING_H_

#include "postprocessing/backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{
  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;






  /** Advanced offset correction on CCD images.
   *
   * Take the averaged of the ccd image and if wanted add a condition.
   *
   * Create an image that is the result of the current image substracted by
   * the averaged image. Where the averaged image is weighted with the sum of
   * all pixels in the current image divided by the sum of all pixels in the
   * averaged image. \n
   * Check which pixels in the resulting image are above a given threshold and
   * the the requested image with One. It follows the function:
   * \f$ Pixel_{resulting Image} +=
   *           (Pixel_{current Image) - \apha \times Pixel_{running Average}\f$
   *
   * @cassttng PostProcessor/\%name\%/{LowerGateEnd|UpperGateEnd} \n
   *           Put only a point into the histogram, when it is in the Gate.
   *           Default is -1e6 ... 1e6
   * @cassttng PostProcessor/\%name\%/{Condition} \n
   *           The PostProcessor we make a condition on. Default is 0.
   * @cassttng PostProcessor/\%name\%/{AveragedImage} \n
   *           The id of the running average of we use for substraction. Default
   *           is id 0.
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   *
   * @author Lutz Foucar
   */
  class pp210 : public PostprocessorBackend
  {
  public:
    /** constructor. */
    pp210(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp210();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings*/
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** the pp that we make the condition on*/
    PostProcessors::key_t _condition;

    /** threshold for peakfinding */
    std::pair<float,float> _gate;

    /** the id of the averaged image */
    PostProcessors::key_t _idAverage;

    /** resulting image */
    Histogram2DFloat *_image;

    /** CCD detector to work on */
    size_t _detector;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;
  };












  /** 1d Histogram of advanced corrected image.
   *
   * A 1D Histogram of all pixels in the corrected image.
   *
   * @cassttng PostProcessor/\%name\%/{Condition} \n
   *           The PostProcessor we make a condition on. Default is 0.
   * @cassttng PostProcessor/\%name\%/{AveragedImage} \n
   *           The id of the running average of we use for substraction. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Condition} \n
   *           The PostProcessor we make a condition on. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   *
   * @author Lutz Foucar
   */
  class pp211 : public PostprocessorBackend
  {
  public:
    /** constructor. */
    pp211(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp211();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings*/
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** the pp that we make the condition on*/
    PostProcessors::key_t _condition;

    /** the id of the averaged image */
    PostProcessors::key_t _idAverage;

    /** resulting 1d Histogram */
    Histogram1DFloat *_hist;

    /** CCD detector to work on */
    size_t _detector;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;
  };


  /** Dump events (from advanced corrected image) to file.
   *
   * Dumps events to file.
   *
   * @cassttng PostProcessor/\%name\%/{Condition} \n
   *           The PostProcessor we make a condition on. Default is 0.
   * @cassttng PostProcessor/\%name\%/{AveragedImage} \n
   *           The id of the running average of we use for substraction. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Condition} \n
   *           The PostProcessor we make a condition on. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PostProcessor/\%name\%/{Filename}\n
   *           The filename to dump events to.
   *           There is no default - you must provide this value.
   *
   * @author Thomas White
   */
  class pp212 : public PostprocessorBackend
  {

  public:
    /** Constructor */
    pp212(PostProcessors& hist, const PostProcessors::key_t &);

    /** Destructor */
    virtual ~pp212();

    /** Dump events */
    virtual void operator()(const CASSEvent &);

    /** Load settings */
    virtual void loadSettings(size_t);

    /** The two histograms that the user wants to subtract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** The PP providing the input */
    PostProcessors::key_t _input;

    /** The PP providing the condition */
    PostProcessors::key_t _condition;

    /** Threshold for peakfinding */
    std::pair<float,float> _gate;

    /** Threshold for coalescing */
    std::pair<float,float> _pregate;

    /** Filename to write events to */
    std::string _filename;

    /** File handle */
    std::ofstream _fh;

    /** Whether or not to coalesce (small) regions into one event */
    bool _coalesce;

    /** Function for recursively checking pixels */
    bool check_pixel(float *f, int x, int y, int w, int h,
                     double &val, int &depth);

  };



  /** Test image
   *
   * @cassttng PostProcessor/\%name\%/{sizeX} \n
   *           Width of testimage (default = 1024)
   * @cassttng PostProcessor/\%name\%/{sizeY} \n
   *           Height of testimage (default = 1024)
   *
   * @author Stephan Kassemeyer
   */
  class pp240 : public PostprocessorBackend
  {
  public:
    /** constructor. */
    pp240(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp240();

    /** create Test image*/
    virtual void operator()(const CASSEvent&);

    /** load the settings*/
    virtual void loadSettings(size_t);

  protected:
    /** resulting image */
    Histogram2DFloat *_image;
    int _sizeX;
    int _sizeY;

  };



}
#endif

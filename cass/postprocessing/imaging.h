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


  /** Dump events (from advanced corrected image) to file.
   *
   * Dumps events to file.
   *
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before filling image.
   *           if this setting is not defined, this postprocessor is unconditional.
   *           Therefore its always true.
   * @cassttng PostProcessor/\%name\%/{AveragedImage} \n
   *           The id of the running average of we use for subtraction. Default
   *           is 0.
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

    /** Dump events */
    virtual void process(const CASSEvent &);

    /** Load settings */
    virtual void loadSettings(size_t);

  protected:
    /** The PP providing the input */
    PostprocessorBackend *_input;

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

    /** create Test image */
    virtual void process(const CASSEvent&);

    /** load the settings*/
    virtual void loadSettings(size_t);

  protected:
    /** Width of testimage */
    int _sizeX;

    /** Height of testimage */
    int _sizeY;

  };



}
#endif

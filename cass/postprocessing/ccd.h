// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"
#include "postprocessor.h"

namespace cass
{

  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;






  /** Last CCD image.
   *
   * Postprocessor will get the raw image from all kinds of ccd's.
   *
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before filling image.
   *           if this setting is not defined, this postprocessor is unconditional.
   * @author Jochen Kuepper
   * @author Lutz Foucar
   */
  class pp100 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp100(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image spcae */
    virtual ~pp100();

    /** dependancy: image retrieval can be conditional */
    virtual PostProcessors::active_t dependencies();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;

    /** current image */
    Histogram2DFloat *_image;

    /** the pp that contains the condition */
    PostProcessors::key_t _condition;

    bool _useCondition;
  };

  /** Integral over the Last CCD image.
   *
   * Postprocessor will get the Integral over the whole image from all kinds of ccd's.
   *
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   * @author Nicola Coppola
   */
  class pp101 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp101(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image spcae */
    virtual ~pp101();

   /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;

    /** Integral of the current image */
    Histogram0DFloat *_ImageIntegral;
  };



  /** Integral over the Last CCD image for pixel above user def Threshold.
   *
   * Postprocessor will get the Integral over the whole image from all kinds of ccd's.
   *
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   * @author Nicola Coppola
   */
  class pp102 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp102(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image spcae */
    virtual ~pp102();

   /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;

    /** Integral of the current image */
    Histogram0DFloat *_ImageIntegralOverThres;
  };






  /** Spectrum of PhotonHits of CCD's.
   *
   * This postprocessor will fill a 1D histogram with the z values in detected
   * Photonhits. Photonhits will be detected in the according preanalyzer.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
   *           properties of the 1D histogram:
   * @cassttng PostProcessor/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PostProcessor/\%name\%/{Adu2eV}\n
   *           conversion factor for converting the z value from ADU to eV.
   *           Default is 1.
   *
   * @author Lutz Foucar
   */
  class pp140 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp140(PostProcessors&, const PostProcessors::key_t&);

    /** destructor */
    virtual ~pp140();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** set the histogram size + retrieve device and detector to work on*/
    virtual void loadSettings(size_t);

  protected:
    /** the Detector that we make the condition on*/
    ACQIRIS::Detectors _conditionDetector;

    /** flag that will invert the condition */
    bool _invert;

    /** range of the photonenergy */
    std::pair<float, float> _range;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** the adu to eV calibration value*/
    float _adu2eV;

    /** current image */
    Histogram1DFloat * _spec;
  };









  /** PhotonHits of CCD's.
   *
   * This postprocessor will fill a 2D histogram with the detected Photonhits.
   * Photonhits will be detected in the pre analyzers. Set the Parameters
   * for detecting photonhits there. (PNCCD::Analyzer or CCD::Analyzer)
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
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
  class pp141 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp141(PostProcessors&, const PostProcessors::key_t&);

    /** destructor */
    virtual ~pp141();

    /** copy pixels from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

  protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** pixel image */
    Histogram2DFloat *_image;
  };

}

#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

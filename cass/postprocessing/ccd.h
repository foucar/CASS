// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "postprocessing/backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

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
   * @cassttng PostProcessor/active/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/active/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   *
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
  };








//  /** Averaged ccd image with optional condition.
//   *
//   * Running average of pnCCD or commercial ccd images. One has the choice to enforce
//   * a condition on the average. This condition is a "tof" detector. The tof detector
//   * will look whether it will find a singal in the assinged channel. If it does the
//   * condition will evaluate to true. You have the option to invert the result of
//   * the evaluation of the condition by setting the invert parameter to true.
//   *
//   * @cassttng PostProcessor/p\%id\%/{average}\n
//   *           how many images should be averaged. Default is 1.
//   * @cassttng PostProcessor/p\%id\%/{ConditionDetector} \n
//   *           Detector that you want to have the condition on. If the detector
//   *           sees a signal than this condition evaluates true. If
//   *           "InvalidDetector" or no Detector is chosen, than the condition
//   *           is not evaluated at all.
//   * @cassttng PostProcessor/p\%id\%/{Invert} \n
//   *           Invert the Condition, when there is a valid detector condition chosen.
//   *           default is "false".
//   * @cassttng PostProcessor/p\%id\%/{bin_horizontal|bin_vertical}\n
//   *           geometric binning (x and y). Binning must be a fraction of 1024 (in
//   *           case of pnccd's) (unused for now)
//   *
//   *
//   * Implements postprocessors 101, 103, 105
//   *
//   * @author Jochen Kuepper
//   * @author Lutz Foucar
//   */
//  class pp101 : public PostprocessorBackend
//  {
//  public:
//    /** constructor */
//    pp101(PostProcessors& hist, PostProcessors::key_t key);
//
//    /** Free _image space */
//    virtual ~pp101();
//
//    /** copy image from CASS event to histogram storage */
//    virtual void operator()(const CASSEvent&);
//
//    /** load the settings for this pp */
//    virtual void loadSettings(size_t);
//
//  protected:
//    /** Length of average */
//    unsigned _average;
//
//    /** Scaling factor of new data to approximate running average */
//    float _scale;
//
//    /** how many pixels to bin in horizontal and vertical direction */
//    std::pair<unsigned, unsigned> _binning;
//
//    /** CCD detector to work on */
//    size_t _detector;
//
//    /** the Detector that we make the condition on*/
//    ACQIRIS::Detectors _conditionDetector;
//
//    /** flag that will invert the condition */
//    bool _invert;
//
//    /** flag telling wether we run for the firsttime*/
//    bool _firsttime;
//
//    /** device the ccd image comes from*/
//    cass::CASSEvent::Device _device;
//
//    /** current image */
//    Histogram2DFloat *_image;
//  };








  /** Spectrum of PhotonHits of CCD's.
   *
   * This postprocessor will fill a 1D histogram with the z values in detected
   * Photonhits. Photonhits will be detected in the according preanalyzer.
   *
   * @cassttng PostProcessor/active/\%name\%/{XNbrBins|XLow|Xup}\n
   *           properties of the 1D histogram:
   * @cassttng PostProcessor/active/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/active/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PostProcessor/active/\%name\%/{Adu2eV}\n
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/active/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PostProcessor/active/\%name\%/{Detector}\n
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






















  /** PhotonHits of CCD's in a 1D histogram.
 *
 * This time energies are plotted in eV and not in arbitrary units
 *
 * This postprocessor will fill a 1d histogram with the z values detected Photonhits.
 * Photonhits will be detected in the commercial Pre Analyzer. They will
 * be just summed up. One needs to clear this histogram, when something has changed.
 *
 * Implements Postprocessor id's: 116, 117, 118
 *
 * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D Histogram:
 *
 * @author Lutz Foucar
 */
  class pp116 : public PostprocessorBackend
  {
  public:
    /** constructor.*/
    pp116(PostProcessors& hist, PostProcessors::key_t key);

    /** Free _image spcae */
    virtual ~pp116();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

  protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** the adu2eV calibration value*/
    double adu2eV;

    /** detector to work on */
    size_t _detector;

    /** current image */
    Histogram1DFloat * _hist;
  };







  ///** @brief Integral of last CCD image (pp3)
  // * @todo fit this to new layout
  // */
  //class pp141 : public PostprocessorBackend
  //{
  //public:
  //
  //    pp141(PostProcessors&, PostProcessors::key_t);
  //
  //    /** Free _image space */
  //    virtual ~pp141();
  //
  //    /** copy image from CASS event to histogram storage
  //
  //    @todo confirm that the simple sum is good enough or whether we need something more accurate
  //    (i.e., Kahan summation, Shewchuk, or similar) (JK, 2010-03-29)
  //    */
  //    virtual void operator()(const CASSEvent&);
  //
  //    /*! Define postprocessor dependency on pp3 (last VMI image) */
  //    virtual PostProcessors::active_t dependencies() {
  //        return PostProcessors::active_t (1, PostProcessors::VmiCcdLastImage); };
  //
  //protected:
  //
  //    Histogram0DFloat *_value;
  //};




}

#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

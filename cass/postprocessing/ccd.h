// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"
#include "postprocessor.h"
#include "pixel_detector_helper.h"


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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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
   */
  class pp100 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp100(PostProcessors&, const PostProcessors::key_t&);

    /** copy image from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;
  };









  /** Integral over the Last CCD image.
   *
   * Postprocessor will get the Integral over the whole image from all kinds of ccd's.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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

    /** process this event */
    virtual void process(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;
  };







  /** Integral over the Last CCD image.
   *
   * Integral is for pixels above user defined threshold. The threshold is
   * defined in the pre analysis of the pnccd or ccd.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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

    /** copy image from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** CCD detector that contains the requested image */
    size_t _detector;

    /** device the ccd image comes from */
    cass::CASSEvent::Device _device;
  };








  /** Spectrum of PhotonHits of CCD's.
   *
   * This postprocessor will fill a 1D histogram with the z values in detected
   * Photonhits. Photonhits will be detected in the according pre analyzer.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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

    /** copy image from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** set the histogram size + retrieve device and detector to work on*/
    virtual void loadSettings(size_t);

  protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** the adu to eV calibration value*/
    float _adu2eV;
  };









  /** PhotonHits of CCD's.
   *
   * This postprocessor will fill a 2D histogram with the detected Photonhits.
   * Photonhits will be detected in the pre analyzers. Set the Parameters
   * for detecting photonhits there. (PNCCD::Analyzer or CCD::Analyzer)
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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

    /** copy pixels from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

  protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;
  };









  /** Spectrum of coalesced PhotonHits of CCD's.
   *
   * This postprocessor will fill a 1D histogram with the z values in detected
   * Photonhits. Photonhits will be detected in the according pre analyzer.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
   *           properties of the 1D histogram:
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the Detector that one is interested in. Default "blubb"
   *
   * @author Lutz Foucar
   */
  class pp142 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp142(PostProcessors&, const PostProcessors::key_t&);

    /** copy pixels from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

  protected:
    /** detector to work on */
    HelperPixelDetectors::instancesmap_t::key_type _detector;

    /** gate on the z */
    std::pair<float,float> _range;
  };




  /** coalesced PhotonHits of CCD's.
   *
   * This postprocessor will fill a 2D histogram with the detected Photonhits.
   * Photonhits will be detected in the pre analyzers. Set the Parameters
   * for detecting photonhits there. (PNCCD::Analyzer or CCD::Analyzer)
   * One can select pixel only to be shown when it has a certain adu.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the Detector that one is interested in. Default "blubb"
   * @cassttng PostProcessor/\%name\%/{UpperLimit|LowerLimit}\n
   *           The range of the z value of the pixel. Only when the pixel is in
   *           this range the pixel will be drawn. Default is 0.0|0.0
   *
   * @author Lutz Foucar
   */
  class pp143 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp143(PostProcessors&, const PostProcessors::key_t&);

    /** copy pixels from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

  protected:
    /** detector to work on */
    HelperPixelDetectors::instancesmap_t::key_type _detector;

    /** gate on the z */
    std::pair<float,float> _range;
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

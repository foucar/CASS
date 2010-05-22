//Copyright (C) 2010 Lutz Foucar

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"

namespace cass
{
  //forward declarations//
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;

  /** Number of Signals in MCP Waveform.
   *
   * This postprocessor will output how many Signals have been found
   * in the acqiris channel for the mcp of the detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 0: InvalidDetector
   *           - 1: HexDetector
   *           - 2: QuadDetector
   *           - 3: VMIMcp
   *           - 4: FELBeamMonitor
   *           - 5: YAGPhotodiode
   *           - 6: FsPhotodiode
   *
   * @author Lutz Foucar
   */
  class pp150 : public PostprocessorBackend
  {
  public:
    /** Constructor. Constructor for Number of Signals*/
    pp150(PostProcessors&, const PostProcessors::key_t&);

    /** Free histogram space */
    virtual ~pp150();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The Histogram storing the info*/
    Histogram0DFloat  *_nbrSignals;
  };








  /** all mcp signals.
   *
   * This postprocessor will output the times of all found singal int the
   * mcp waveform of the detector. This is a Time of Flight Spectrum
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 0: InvalidDetector
   *           - 1: HexDetector
   *           - 2: QuadDetector
   *           - 3: VMIMcp
   *           - 4: FELBeamMonitor
   *           - 5: YAGPhotodiode
   *           - 6: FsPhotodiode
   *
   * @author Lutz Foucar
   */
  class pp151 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp151(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp151();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The Histogram storing the info*/
    Histogram1DFloat  *_tof;
  };







  /** FWHM vs. Height of mcp signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * found mcp signals.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 0: InvalidDetector
   *           - 1: HexDetector
   *           - 2: QuadDetector
   *           - 3: VMIMcp
   *           - 4: FELBeamMonitor
   *           - 5: YAGPhotodiode
   *           - 6: FsPhotodiode
   *
   * @author Lutz Foucar
   */
  class pp152 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp152(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp152();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The Histogram storing the info*/
    Histogram2DFloat  *_sigprop;
  };








  /** Number of Signals in Anode Layers Waveform.
   *
   * This postprocessor will output how many Signals have been found in the
   * acqiris channels of requested layers.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{Wireend}\n
   *           The anode layer Wireend. Default is 1. Options are:
   *           - 1: first wireend
   *           - 2: second wireend
   *
   * @author Lutz Foucar
   */
  class pp160 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp160(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp160();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The layer of the detector detector we are there for*/
    char _layer;

    /** The Signal of the layer detector we are there for*/
    char _signal;

    /** The Histogram storing the info*/
    Histogram0DFloat  *_nbrSignals;
  };











  /** FWHM vs. Height of wireend signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * all identified signals in a detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{Wireend}\n
   *           The anode layer Wireend. Default is 1. Options are:
   *           - 1: first wireend
   *           - 2: second wireend
   *
   * @author Lutz Foucar
   */
  class pp161 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp161(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp161();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The layer of the detector detector we are there for*/
    char _layer;

    /** The Signal of the layer detector we are there for*/
    char _signal;

    /** The Histogram storing the info*/
    Histogram2DFloat  *_sigprop;
  };










  /** Timesum of Delayline.
   *
   * This postprocessor will output Timesum of a Delayline Anode for the first
   * hit in a selectable good range.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   *
   * @author Lutz Foucar
   */
  class pp162 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp162(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp162();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The layer of the detector detector we are there for*/
    char _layer;

    /** The Histogram storing the info*/
    Histogram0DFloat  *_timesum;
  };








  /** Timesum of Delayline Anode vs Position of Anode.
   *
   * This postprocessor will output Timesum of a Delayline Anode versus the
   * position of the delayline. This is used to know the value for extracting
   * the detectorhits.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   *
   * @author Lutz Foucar
   */
  class pp163 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp163(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp163();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The layer of the detector detector we are there for*/
    char _layer;

    /** The Histogram storing the info*/
    Histogram2DFloat  *_timesumvsPos;
  };










  /** detector picture of first hit.
   *
   * This postprocessor will output the Detector picture of the first Hit in
   * the selectable good range. The added Hit fullfilles the timesum condition.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{FirstLayer}\n
   *           The anode layer of the first coordinate. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{SecondLayer}\n
   *           The anode layer of the second coordinate. Default is V. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   *
   * @author Lutz Foucar
   */
  class pp164 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp164(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp164();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The first layer of the detector for the position */
    char _first;

    /** The second layer of the detector for the position */
    char _second;

    /** The Histogram storing the info*/
    Histogram2DFloat  *_pos;
  };










  /** Number of reconstucted hits.
   *
   * This postprocessor will output the number of reconstructed detector hits.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   *
   * @author Lutz Foucar
   */
  class pp165 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
    pp165(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp165();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The Histogram storing the info*/
    Histogram0DFloat  *_nbrHits;
  };










  /** detector hits values.
   *
   * This postprocessor will output the Detector Hit values reqeuested.
   * depending on the postprocessor id, it will histogram 2 of the 3 values
   * of an detectorhit. It will make a condition on the third value of the
   * detector hit.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           The detector that we are responsible for. Default is 1. Options are:
   *           - 1: HexDetector
   *           - 2: QuadDetector
   * @cassttng PostProcessor/\%name\%/{XInput}\n
   *           The value that should be put onto the x-axis of the histogram.
   *           Default is 'x'. Options are:
   *           - x: x-position of the reconstructed hit
   *           - y: x-position of the reconstructed hit
   *           - t: time of impact of the reconstructed hit
   * @cassttng PostProcessor/\%name\%/{YInput}\n
   *           The value that should be put onto the x-axis of the histogram.
   *           Default is 'y'. Options are:
   *           - x: x-position of the reconstructed hit
   *           - y: x-position of the reconstructed hit
   *           - t: time of impact of the reconstructed hit
   * @cassttng PostProcessor/\%name\%/{ConditionLow|ConditionHigh}\n
   *           conditions on third value, the one not chosen with options above.
   *
   * @author Lutz Foucar
   */
  class pp166 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp166(PostProcessors&, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp166();

    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;

    /** The first value of the detector hit */
    char _first;

    /** The second value of the detector */
    char _second;

    /** The third value of the detector, that we will check the condition for*/
    char _third;

    /** The condition that we impose on the third component*/
    std::pair<float, float> _condition;

    /** The Histogram storing the info*/
    Histogram2DFloat  *_hist;
  };














  /** Pipico spectra.
   *
   * This postprocessor will create Photo Ion Photo Ion Coincidence Spectra.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p%id%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   *
   * implements postprocessor id's: 700, 701
   *
   * @author Lutz Foucar
   */
  class pp700 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp700(PostProcessors&, PostProcessors::key_t key);
    /** Free _image space */
    virtual ~pp700();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);
  protected:
    /** The first detector of the cooincdence*/
    ACQIRIS::Detectors _detector01;
    /** The second detector of the cooincdence*/
    ACQIRIS::Detectors _detector02;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_pipico;
  };



}//end cass

#endif

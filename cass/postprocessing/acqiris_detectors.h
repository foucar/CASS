//Copyright (C) 2010 Lutz Foucar

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"

namespace cass
{
  //forward declarations//
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's 550, 600, 650, 660, 670, 680.
   *
   * @author Lutz Foucar
   */
  class pp550 : public PostprocessorBackend
  {
  public:
    /** Constructor. Constructor for Number of Signals*/
    pp550(PostProcessors &, PostProcessors::id_t);
    /** Free histogram space */
    virtual ~pp550();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_nbrSignals;
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's: 551-556 & 601-604
   *
   * @author Lutz Foucar
   */
  class pp551 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp551(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp551();
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
    Histogram1DFloat  *_nbrSignals;
  };



  /** Signal ratio for two layers.
   *
   * This postprocessor will output the Ratio of the Number of Signals with
   * Respect to the other layer.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's: 577, 560, 563, 605, 608
   *
   * @author Lutz Foucar
   */
  class pp557 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the Signals of two Layers*/
    pp557(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp557();
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
    Histogram1DFloat  *_ratio;
  };



  /** Signal Ratio of wireend vs mcp.
   *
   * This postprocessor will output the Ratio of the Number of Signals with
   * Respect to the the number of Singals in the MCP Channel.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's: 558-559, 561-562, 564-565, 606-507, 609-610
   *
   * @author Lutz Foucar
   */
  class pp558 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the Signals of Layers vs MCP*/
    pp558(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp558();
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
    char _wireend;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };




  /** Ratio of reconstucted hits vs mcp hits.
   *
   * This postprocessor will output the Ratio of the Number reconstructed
   * detector hits with respect to the the number of Signals in the mcp channel.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's: 566, 611
   *
   * @author Lutz Foucar
   */
  class pp566 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
    pp566(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp566();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };





  /** all mcp hits.
   *
   * This postprocessor will output the times of all found Hits on the Mcp.
   * This is more or less a Time of Flight Spektrum
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   *
   * implements postprocessor id's: 567, 612, 651, 661, 671, 681
   *
   * @author Lutz Foucar
   */
  class pp567 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp567(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp567();
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
   * @cassttng PostProcessor/p\%id%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   *
   * implements postprocessor id's: 568-570, 613, 614
   *
   * @author Lutz Foucar
   */
  class pp568 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp568(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp568();
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
    Histogram1DFloat  *_timesum;
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   *
   * implements postprocessor id's: 571-573, 615, 616
   *
   * @author Lutz Foucar
   */
  class pp571 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp571(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp571();
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   *
   * implements postprocessor id's: 574-577, 617
   *
   * @author Lutz Foucar
   */
  class pp574 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp574(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp574();
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
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/p\%id\%/{ConditionLow|ConditionHigh}\n
   *           conditions on third value
   *
   * implements postprocessor id's: 578-580, 618-620
   *
   * @author Lutz Foucar
   */
  class pp578 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp578(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp578();
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







  /** FWHM vs. Height of mcp signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * all identified signals in a detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p\%id\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   *
   * implements postprocessor id's: 581, 621, 652, 662, 672, 682
   *
   * @author Lutz Foucar
   */
  class pp581 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp581(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp581();
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



  /** FWHM vs. Height of wireend signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * all Wireend Signals in a delaylinedetector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @cassttng PostProcessor/p%id%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   *
   * implements postprocessor id's: 582-587, 622-625
   *
   * @author Lutz Foucar
   */
  class pp582 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp582(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp582();
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
    pp700(PostProcessors&, PostProcessors::id_t);
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

//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors.h file contains declaration of postprocessors that
 *                           extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"
#include "acqiris_detectors_helper.h"
#include "signal_producer.h"
#include "delayline_detector.h"


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
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the detector that we work on. Default is "blubb"
   *
   * @author Lutz Foucar
   */
  class pp150 : public PostprocessorBackend
  {
  public:
    /** Constructor. Constructor for Number of Signals*/
    pp150(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
  };








  /** all mcp signals.
   *
   * This postprocessor will output the times of all found singal int the
   * mcp waveform of the detector. This is a Time of Flight Spectrum
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the detector that we work on. Default is "blubb"
   *
   * @author Lutz Foucar
   */
  class pp151 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp151(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
  };







  /** FWHM vs. Height of mcp signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * found mcp signals.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector or cass::ACQIRIS:::TofDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the detector that we work on. Default is "blubb"
   *
   * @author Lutz Foucar
   */
  class pp152 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp152(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
  };








  /** Number of Signals in Anode Layers Waveform.
   *
   * This postprocessor will output how many Signals have been found in the
   * acqiris channels of requested layers.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
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

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The layer of the detector detector we are there for*/
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type  _layer;

    /** The Signal of the layer detector we are there for*/
    ACQIRIS::AnodeLayer::wireends_t::key_type _signal;
  };











  /** FWHM vs. Height of wireend signals.
   *
   * This postprocessor will make a histogram of the fwhm and height of
   * all identified signals in a detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
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

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The layer of the detector detector we are there for*/
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type  _layer;

    /** The Signal of the layer detector we are there for*/
    ACQIRIS::AnodeLayer::wireends_t::key_type _signal;
  };










  /** Timesum of Delayline.
   *
   * This postprocessor will output Timesum of a Delayline Anode for the first
   * hit in a selectable good range.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - if detector type is HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - if detector type is Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
   *           The time range in which we will take the first hits on the wireends
   *           and the mcp signal to calculate the timesum. This should be a
   *           timerange in which mostly single detectorhits are to be expected.
   *           Default is 0 | 20000.
   *
   * @author Lutz Foucar
   */
  class pp162 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp162(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The layer of the detector detector we are there for*/
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type _layer;

    /** the range in which the single events are in */
    std::pair<double,double> _range;
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
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
   * @cassttng PostProcessor/\%name\%/{Layer}\n
   *           The anode layer. Default is U. Options are:
   *           - if detector type is HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - if detector type is Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
   *           The time range in which we will take the first hits on the wireends
   *           and the mcp signal to calculate the timesum. This should be a
   *           timerange in which mostly single detectorhits are to be expected.
   *           Default is 0 | 20000.
   *
   * @author Lutz Foucar
   */
  class pp163 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp163(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The layer of the detector detector we are there for*/
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type _layer;

    /** the range in which the single events are in */
    std::pair<double,double> _range;
  };










  /** detector picture of first hit.
   *
   * This postprocessor will output the Detector picture of the first Hit in
   * the selectable good range. The added Hit fullfilles the timesum condition.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
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
   *           The anode layer of the second coordinate. Default is U. Options are:
   *           - for HexDetector
   *             - U: U-Layer
   *             - V: V-Layer
   *             - W: W-Layer
   *           - for Quad Detector
   *             - X: X-Layer
   *             - Y: Y-Layer
   * @cassttng PostProcessor/\%name\%/{TimesumFirstLayerLow|TimesumFirstLayerHigh}\n
   *           The Timesumcondition range for the first layer.
   *           Default is 20 | 200.
   * @cassttng PostProcessor/\%name\%/{TimesumSecondLayerLow|TimesumSecondLayerHigh}\n
   *           The Timesumcondition range for the Second layer.
   *           Default is 20 | 200.
   * @cassttng PostProcessor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
   *           The time range in which we will take the first hits on the wireends
   *           and the mcp signal to calculate the timesum. This should be a
   *           timerange in which mostly single detectorhits are to be expected.
   *           Default is 0 | 20000.
   *
   * @author Lutz Foucar
   */
  class pp164 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp164(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The first layer of the detector for the position */
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type _first;

    /** The second layer of the detector for the position */
    ACQIRIS::DelaylineDetector::anodelayers_t::key_type _second;

    /** the range in which the single events are in */
    std::pair<double,double> _range;

    /** timesum ranges of the layers */
    std::pair<std::pair<double, double>,
              std::pair<double, double> > _tsrange;
  };










  /** Number of reconstucted hits.
   *
   * This postprocessor will output the number of reconstructed detector hits.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::SignalProducer
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
   *
   * @author Lutz Foucar
   */
  class pp165 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
    pp165(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the delayline detector that we work on. Default is "blubb"
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
   * @cassttng PostProcessor/\%name\%/{ConditionInput}\n
   *           The value that should be checked for the range before filling the
   *           histogram. Default is 't'. Options are:
   *           - x: x-position of the reconstructed hit
   *           - y: x-position of the reconstructed hit
   *           - t: time of impact of the reconstructed hit
   * @cassttng PostProcessor/\%name\%/{ConditionLow|ConditionHigh}\n
   *           the condition range that should be applied to the condition input
   *
   * @author Lutz Foucar
   */
  class pp166 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp166(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** The first value of the detector hit */
    ACQIRIS::SignalProducer::signal_t::key_type _first;

    /** The second value of the detector */
    ACQIRIS::SignalProducer::signal_t::key_type _second;

    /** The third value of the detector, that we will check the condition for*/
    ACQIRIS::SignalProducer::signal_t::key_type _third;

    /** The condition that we impose on the third component*/
    std::pair<float, float> _cond;
  };














  /** Pipico spectra.
   *
   * This postprocessor will create Photo-Ion Photo-Ion Coincidence Spectra.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{FirstDetector}\n
   *           Name of the first detector that we work on. Default is "blubb"
   * @cassttng PostProcessor/\%name\%/{SecondDetector}\n
   *           Name of the first detector that we work on. Default is "blubb"
   *
   * @author Lutz Foucar
   */
  class pp220 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp220(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The first detector of the cooincdence*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector01;

    /** The second detector of the cooincdence*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector02;
  };







  /** Particle value.
   *
   * retrieve one particle property from a Particle that belong to a detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the first detector that we work on. Default is "blubb"
   * @cassttng PostProcessor/\%name\%/{Particle}\n
   *           Name of the particle whos properties we want to extract
   *           Default is "NeP"
   * @cassttng PostProcessor/\%name\%/{Property}\n
   *           Name of the particles first property we want to extract
   *           Default is "px".
   *           Available Properties are:
   *           - raw and corrected detectorhit values
   *             - x_mm (raw value of detectorhit)
   *             - y_mm (raw value of detectorhit)
   *             - tof_ns (raw value of detectorhit)
   *             - xCor_mm (corrected position)
   *             - yCor_mm (corrected position)
   *             - xCorScal_mm (corrected and scaled position)
   *             - yCorScal_mm (corrected and scaled position)
   *             - xCorScalRot_mm (corrected, scaled and rotated position)
   *             - yCorScalRot_mm (corrected, scaled and rotated position)
   *             - tofCor_ns (corrected time of flight)
   *           - kartesian coordinates representation of momenta
   *             - px
   *             - py
   *             - pz
   *           - polar coordinate representation of momenta
   *             - roh
   *             - theta
   *             - phi
   *           - Energy
   *             - e_au
   *             - e_eV
   *
   * @author Lutz Foucar
   */
  class pp250 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp250(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** the particle we are working on */
    ACQIRIS::DelaylineDetector::particles_t::key_type _particle;

    /** the property of the particle that we want to retrieve */
    ACQIRIS::particleHit_t::key_type _property;
  };









  /** Particle values.
   *
   * create 2d hist of two particle properties from a Particle that belong to a
   * detector.
   *
   * To set up the channel assignment for the requested detector one needs to set
   * up the detector parameters.
   * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
   *      cass::ACQIRIS::Signal and cass::ACQIRIS::Particle
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{Detector}\n
   *           Name of the detector that the particles belong to.
   *           Default is "blubb"
   * @cassttng PostProcessor/\%name\%/{Particle}\n
   *           Name of the particle whos properties we want to extract
   *           Default is "NeP"
   * @cassttng PostProcessor/\%name\%/{FirstProperty}\n
   *           Name of the particles first property we want to extract
   *           Default is "px".
   *           Available Properties are:
   *           - raw and corrected detectorhit values
   *             - x_mm (raw value of detectorhit)
   *             - y_mm (raw value of detectorhit)
   *             - tof_ns (raw value of detectorhit)
   *             - xCor_mm (corrected position)
   *             - yCor_mm (corrected position)
   *             - xCorScal_mm (corrected and scaled position)
   *             - yCorScal_mm (corrected and scaled position)
   *             - xCorScalRot_mm (corrected, scaled and rotated position)
   *             - yCorScalRot_mm (corrected, scaled and rotated position)
   *             - tofCor_ns (corrected time of flight)
   *           - kartesian coordinates representation of momenta
   *             - px
   *             - py
   *             - pz
   *           - polar coordinate representation of momenta
   *             - roh
   *             - theta
   *             - phi
   *           - Energy
   *             - e_au
   *             - e_eV
   * @cassttng PostProcessor/\%name\%/{SecondProperty}\n
   *           Name of the particles second property we want to extract
   *           Default is "py"
   *           Available Properties are:
   *           - raw and corrected detectorhit values
   *             - x_mm (raw value of detectorhit)
   *             - y_mm (raw value of detectorhit)
   *             - tof_ns (raw value of detectorhit)
   *             - xCor_mm (corrected position)
   *             - yCor_mm (corrected position)
   *             - xCorScal_mm (corrected and scaled position)
   *             - yCorScal_mm (corrected and scaled position)
   *             - xCorScalRot_mm (corrected, scaled and rotated position)
   *             - yCorScalRot_mm (corrected, scaled and rotated position)
   *             - tofCor_ns (corrected time of flight)
   *           - kartesian coordinates representation of momenta
   *             - px
   *             - py
   *             - pz
   *           - polar coordinate representation of momenta
   *             - roh
   *             - theta
   *             - phi
   *           - Energy
   *             - e_au
   *             - e_eV
   *
   * @author Lutz Foucar
   */
  class pp251 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp251(PostProcessors&, const PostProcessors::key_t&);

    /** Retrieve the number of Signals and histogram it */
    virtual void process(const CASSEvent&);

    /** load the histogram settings from file*/
    virtual void loadSettings(size_t);

  protected:
    /** The detector we are there for*/
    ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

    /** the particle we are working on */
    ACQIRIS::DelaylineDetector::particles_t::key_type _particle;

    /** the first property of the particle that we want to retrieve */
    ACQIRIS::particleHit_t::key_type _property01;

    /** the second property of the particle that we want to retrieve */
    ACQIRIS::particleHit_t::key_type _property02;
  };







}//end cass

#endif

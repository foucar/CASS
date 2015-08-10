//Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file acqiris_detectors.h file contains declaration of processors that
 *                           extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "processor.h"
#include "acqiris_detectors_helper.h"
#include "signal_producer.h"
#include "delayline_detector.h"
#include "result.hpp"


namespace cass
{
/** Number of Signals in MCP Waveform.
 *
 * @PPList "150":TofDetector number of signals in MCP waveform
 *
 * This processor will output how many Signals have been found
 * in the acqiris channel for the mcp of the detector.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp150 : public Processor
{
public:
  /** Constructor. Constructor for Number of Signals*/
  pp150(const name_t&);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &res);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
};








/** all mcp signals.
 *
 * @PPList "151":TofDetector all signals
 *
 * This processor will output the times of all found singal int the
 * mcp waveform of the detector. This is a Time of Flight Spectrum
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the 1d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp151 : public Processor
{
public:
  /** Constructor*/
  pp151(const name_t&);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &res);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
};







/** FWHM vs. Height of mcp signals.
 *
 * @PPList "152":TofDetector signal height vs. fwhm
 *
 * This processor will make a histogram of the fwhm and height of
 * found mcp signals.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector or cass::ACQIRIS:::TofDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp152 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp152(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &res);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
};








/** Deadtime between two consecutive mcp signals
 *
 * @PPList "153":TofDetector Deadtime between two consecutive MCP signals
 *
 * This processor will output a histogram of the deadtime between two
 * consecutive mcp signals.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp153 : public Processor
{
public:
  /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
  pp153(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
};













/** Number of Signals in Anode Layers Waveform.
 *
 * @PPList "160":Delayline wireend number of signals
 *
 * This processor will output how many Signals have been found in the
 * acqiris channels of requested layers.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Layer}\n
 *           The anode layer. Default is U. Options are:
 *           - for HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - for Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{Wireend}\n
 *           The anode layer Wireend. Default is 1. Options are:
 *           - 1: first wireend
 *           - 2: second wireend
 *
 * @author Lutz Foucar
 */
class pp160 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp160(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "161":Delayline wireend signal height vs. fwhm
 *
 * This processor will make a histogram of the fwhm and height of
 * all identified signals in a detector.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Layer}\n
 *           The anode layer. Default is U. Options are:
 *           - for HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - for Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{Wireend}\n
 *           The anode layer Wireend. Default is 1. Options are:
 *           - 1: first wireend
 *           - 2: second wireend
 *
 * @author Lutz Foucar
 */
class pp161 : public Processor
{
public:
  /** Constructor*/
  pp161(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "162":Delayline timesum on anode
 *
 * This processor will output Timesum of a Delayline Anode for the first
 * hit in a selectable good range.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Layer}\n
 *           The anode layer. Default is U. Options are:
 *           - if detector type is HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - if detector type is Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
 *           The time range in which we will take the first hits on the wireends
 *           and the mcp signal to calculate the timesum. This should be a
 *           timerange in which mostly single detectorhits are to be expected.
 *           Default is 0 | 20000.
 *
 * @author Lutz Foucar
 */
class pp162 : public Processor
{
public:
  /** Constructor*/
  pp162(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "163":Delayline timesum on anode vs. position
 *
 * This processor will output Timesum of a Delayline Anode versus the
 * position of the delayline. This is used to know the value for extracting
 * the detectorhits.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Layer}\n
 *           The anode layer. Default is U. Options are:
 *           - if detector type is HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - if detector type is Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
 *           The time range in which we will take the first hits on the wireends
 *           and the mcp signal to calculate the timesum. This should be a
 *           timerange in which mostly single detectorhits are to be expected.
 *           Default is 0 | 20000.
 *
 * @author Lutz Foucar
 */
class pp163 : public Processor
{
public:
  /** Constructor */
  pp163(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "164":Delayline image of first good hit
 *
 * This processor will output the Detector picture of the first Hit in
 * the selectable good range. The added Hit fullfilles the timesum condition.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{FirstLayer}\n
 *           The anode layer of the first coordinate. Default is U. Options are:
 *           - for HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - for Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{SecondLayer}\n
 *           The anode layer of the second coordinate. Default is U. Options are:
 *           - for HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - for Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{TimesumFirstLayerLow|TimesumFirstLayerHigh}\n
 *           The Timesumcondition range for the first layer.
 *           Default is 20 | 200.
 * @cassttng Processor/\%name\%/{TimesumSecondLayerLow|TimesumSecondLayerHigh}\n
 *           The Timesumcondition range for the Second layer.
 *           Default is 20 | 200.
 * @cassttng Processor/\%name\%/{TimeRangeLow|TimeRangeHigh}\n
 *           The time range in which we will take the first hits on the wireends
 *           and the mcp signal to calculate the timesum. This should be a
 *           timerange in which mostly single detectorhits are to be expected.
 *           Default is 0 | 20000.
 *
 * @author Lutz Foucar
 */
class pp164 : public Processor
{
public:
  /** Constructor */
  pp164(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "165":Delayline reconstructed Number of detectorhits
 *
 * This processor will output the number of reconstructed detector hits.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp165 : public Processor
{
public:
  /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
  pp165(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
};










/** detector hits values.
 *
 * @PPList "166":Delayline data of all reconstructed detectorhits
 *
 * This processor will output the Detector Hit values reqeuested.
 * depending on the processor id, it will histogram 2 of the 3 values
 * of an detectorhit. It will make a condition on the third value of the
 * detector hit.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::Signal
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{XInput}\n
 *           The value that should be put onto the x-axis of the histogram.
 *           Default is 0. For all options see ACQIRIS::detectorHits
 * @cassttng Processor/\%name\%/{YInput}\n
 *           The value that should be put onto the x-axis of the histogram.
 *           Default is 1. For all options see ACQIRIS::detectorHits
 * @cassttng Processor/\%name\%/{ConditionInput}\n
 *           The value that should be checked for the range before filling the
 *           histogram.
 *           Default is 2. For all options see ACQIRIS::detectorHits
 * @cassttng Processor/\%name\%/{ConditionLow|ConditionHigh}\n
 *           the condition range that should be applied to the condition input
 *
 * @author Lutz Foucar
 */
class pp166 : public Processor
{
public:
  /** Constructor */
  pp166(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

  /** The first value of the detector hit */
  ACQIRIS::detectorHits _first;

  /** The second value of the detector */
  ACQIRIS::detectorHits _second;

  /** The third value of the detector, that we will check the condition for*/
  ACQIRIS::detectorHits _third;

  /** The condition that we impose on the third component*/
  std::pair<float, float> _cond;
};





















/** Deadtime between two consecutive anode signals
 *
 * @PPList "167":Delayline Deadtime between two consecutive anode signals
 *
 * This processor will output a histogram of the deadtime between two
 * consecutive anode signals.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Layer}\n
 *           The anode layer. Default is U. Options are:
 *           - for HexDetector
 *             - U: U-Layer
 *             - V: V-Layer
 *             - W: W-Layer
 *           - for Quad Detector
 *             - X: X-Layer
 *             - Y: Y-Layer
 * @cassttng Processor/\%name\%/{Wireend}\n
 *           The anode layer Wireend. Default is 1. Options are:
 *           - 1: first wireend
 *           - 2: second wireend
 *
 * @author Lutz Foucar
 */
class pp167 : public Processor
{
public:
  /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
  pp167(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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














/** Pipico spectra.
 *
 * @PPList "220":PIPICO Spectrum
 *
 * This processor will create Photo-Ion Photo-Ion Coincidence Spectra.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::Signal
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{FirstDetector}\n
 *           Name of the first detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{SecondDetector}\n
 *           Name of the first detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 *
 * @author Lutz Foucar
 */
class pp220 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp220(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

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
 * @PPList "250":Property of particle
 *
 * retrieve one particle property from a Particle that belong to a detector.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::Signal
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the 1d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the first detector that we work on. Default is "blubb"
 * @cassttng Processor/\%name\%/{Particle}\n
 *           Name of the particle whos properties we want to extract
 *           Default is "NeP"
 * @cassttng Processor/\%name\%/{Property}\n
 *           Name of the particles first property we want to extract
 *           Default is 0.
 *           Available Properties see ACQIRIS::particleHits
 *
 * @author Lutz Foucar
 */
class pp250 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp250(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

  /** the particle we are working on */
  ACQIRIS::DelaylineDetector::particles_t::key_type _particle;

  /** the property of the particle that we want to retrieve */
  ACQIRIS::particleHits _property;
};









/** Particle values.
 *
 * @PPList "251":2d hist with two properties of particle
 *
 * create 2d hist of two particle properties from a Particle that belong to a
 * detector.
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::Signal and cass::ACQIRIS::Particle
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the detector that the particles belong to.
 *           Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Particle}\n
 *           Name of the particle whos properties we want to extract
 *           Default is "NeP"
 * @cassttng Processor/\%name\%/{FirstProperty}\n
 *           Name of the particles first property we want to extract
 *           Default is 0.
 *           Available Properties see ACQIRIS::particleHits
 * @cassttng Processor/\%name\%/{SecondProperty}\n
 *           Name of the particles second property we want to extract
 *           Default is 1
 *           Available Properties see ACQIRIS::particleHits
 *
 * @author Lutz Foucar
 */
class pp251 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp251(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

  /** the particle we are working on */
  ACQIRIS::DelaylineDetector::particles_t::key_type _particle;

  /** the first property of the particle that we want to retrieve */
  ACQIRIS::particleHits _property01;

  /** the second property of the particle that we want to retrieve */
  ACQIRIS::particleHits _property02;
};





/** Number of particles found per shot.
 *
 * @PPList "252":Number of found particle hits per shot
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::Signal
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the 1d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the first detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors as starting point for
 *           more information on how to set up the different types of Acqiris
 *           type detectors.
 * @cassttng Processor/\%name\%/{Particle}\n
 *           Name of the particle whos properties we want to extract
 *           Default is "NeP"
 *
 * @author Lutz Foucar
 */
class pp252 : public Processor
{
public:
  /** Constructor for Number of Signals*/
  pp252(const name_t &);

  /** Retrieve the number of Signals and histogram it */
  virtual void process(const CASSEvent&, result_t &);

  /** load the histogram settings from file*/
  virtual void loadSettings(size_t);

protected:
  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

  /** the particle we are working on */
  ACQIRIS::DelaylineDetector::particles_t::key_type _particle;
};




}//end namespace cass

#endif

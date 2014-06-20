// Copyright (C) 2010 Lutz Foucar

/**
 * @file waveform.h file contains acqiris data retrieval postprocessor
 *                  declaration
 *
 * @author Lutz Foucar
 */

#ifndef _WAVEFORM_POSTPROCESSOR_H_
#define _WAVEFORM_POSTPROCESSOR_H_

#include "processor.h"
#include "cass_acqiris.h"

namespace cass
{
//forward declarations
class CASSEvent;
class Histogram1DFloat;

/** acqiris channel waveform.
 *
 * @PPList "110": retrieve wavefrom of a channel
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{InstrumentId} \n
 *           The instrument id of the acqiris instrument that contains the
 *           channel. Default is 8. Options are:
 *           - 8: Camp Acqiris
 *           - 4: AMO ITof Acqiris
 *           - 2: AMO GD Acqiris
 *           - 5: AMO Mbes Acqiris
 *           - 22: XPP Acqiris
 * @cassttng PostProcessor/\%name\%/{ChannelNbr} \n
 *           The channel number of the acqiris instrument. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrSamples} \n
 *           Number of samples in the waveform. Default is 40000
 * @cassttng PostProcessor/\%name\%/{SampleInterval} \n
 *           Sample Interval (Time between to samples in s. Default is 1e-9
 *
 * @author Lutz Foucar
 */
class pp110 : public PostProcessor
{
public:
  /** constructor */
  pp110(const name_t &name);

  /** copy the last waveform from the channel*/
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the instrument that contains the channel this postprocessor will work on */
  ACQIRIS::Instruments _instrument;

  /** the Acqiris channel number of this processor */
  size_t _channel;

  /** the sample interval */
  double _sampleInterval;
};



/** cfd trace from waveform
 *
 * @PPList "111": convert wavefrom to cfd trace
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Waveform} \n
 *           The name of the PostProcessor containing the waveform that should
 *           be converted. Default is 'Unknown'
 * @cassttng PostProcessor/\%name\%/{Delay_ns}\n
 *           Delay in ns used. Default is 5.
 * @cassttng PostProcessor/\%name\%/{Fraction}\n
 *           Fraction used. Default is 0.6
 * @cassttng PostProcessor/\%name\%/{Walk_V}\n
 *           walk in V used. Default is 0.
 *
 * @author Lutz Foucar
 */
class pp111 : public PostProcessor
{
public:
  /** constructor */
  pp111(const name_t &name);

  /** copy the last waveform from the channel*/
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the PostProcessor that contains the waveform to convert */
  shared_pointer _waveform;

  /** the delay in bins */
  size_t _delay;

  /** the fraction */
  float _fraction;

  /** the walk in volts */
  float _walk;
};
}

#endif

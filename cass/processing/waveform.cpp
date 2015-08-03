//Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file waveform.cpp file contains acqiris data retrieval processor
 *                    definition
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>
#include <tr1/functional>

#include "waveform.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass.h"
#include "acqiris_device.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"
#include "cass_exceptions.hpp"

using namespace cass;
using namespace ACQIRIS;
using std::runtime_error;
using std::logic_error;
using std::minus;
using std::multiplies;
using std::cout;
using std::endl;
using std::invalid_argument;
using std::tr1::shared_ptr;
using std::tr1::bind;
using std::tr1::placeholders::_1;

//the last wavefrom processor
pp110::pp110(const name_t &name)
  :Processor(name)
{
  loadSettings(0);
}

void pp110::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _instrument = static_cast<Instruments>(s.value("InstrumentId",8).toUInt());
  _channel    = s.value("ChannelNbr",0).toUInt();
  int wsize(s.value("NbrSamples",40000).toInt());
  _sampleInterval = s.value("SampleInterval",1e-9).toDouble();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(
        shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(wsize,0,wsize*_sampleInterval,"Time [s]")));
  Log::add(Log::INFO,"Processor '" + name() + "' is showing channel '" +
           toString(_channel) + "' of acqiris '" + toString(_instrument) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp110::process(const CASSEvent &evt, HistogramBackend &res)
{
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));
  const Device &dev
      (dynamic_cast<const Device&>(*(evt.devices().find(CASSEvent::Acqiris)->second)));
  Device::instruments_t::const_iterator instrIt (dev.instruments().find(_instrument));
  if (dev.instruments().end() == instrIt)
    throw logic_error("pp110::process() '" + name() +
                        "': Data doesn't contain Instrument '"+toString(_instrument)
                        + "'");
  const Instrument &instr(instrIt->second);
  if (instr.channels().size() <= _channel)
    throw runtime_error("pp110::process() '" + name() + "': Instrument '"+
                        toString(_instrument) + "' doesn't contain channel '" +
                        toString(_channel)+ "'");
  const Channel &channel (instr.channels()[_channel]);
  const waveform_t &waveform (channel.waveform());
  if (result.axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming waveforms NbrSamples '" + toString(waveform.size()) +
                           "'. User set NbrSamples '" +
                           toString(result.axis()[HistogramBackend::xAxis].nbrBins()) +
                           "'");
  }
  if (!std::isfinite(channel.gain()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided gain '" + toString(channel.gain()) +
                      "' is not a number");
  }
  if (!std::isfinite(channel.sampleInterval()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided sampleInterval '" +
                      toString(channel.sampleInterval()) + "' is not a number");
  }
  if (!std::isfinite(channel.offset()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided vertical offset '" +
                      toString(channel.offset()) + "' is not a number");
  }
  if (!(std::abs(channel.sampleInterval()-_sampleInterval) < sqrt(std::numeric_limits<double>::epsilon())))
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming waveforms SampleInterval '" + toString(channel.sampleInterval()) +
                           "'. User set SampleInterval '" + toString(_sampleInterval) + "'");
  }
  transform(waveform.begin(), waveform.end(),
            result.memory().begin(),
            bind(minus<float>(),
                 bind(multiplies<float>(),channel.gain(),_1),channel.offset()));
  result.nbrOfFills()=1;
}




// ***cfd trace from waveform

pp111::pp111(const name_t &name)
  :Processor(name)
{
  loadSettings(0);
}

void pp111::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _waveform = setupDependency("Waveform");
  setupGeneral();
  if (!setupCondition())
    return;
  if (_waveform->result().dimension() != 1)
    throw invalid_argument("pp111 '" + name() + "' histogram '" + _waveform->name() +
                           "' is not a 1D histogram");

  _fraction = s.value("Fraction",0.6).toFloat();
  _walk = s.value("Walk_V",0).toFloat();
  const float delay(s.value("Delay_ns",5).toFloat());
  const size_t nBins(_waveform->result().axis()[Histogram1DFloat::xAxis].nbrBins());
  const float Up(_waveform->result().axis()[Histogram1DFloat::xAxis].upperLimit());
  const float samplInter(Up/nBins);
  _delay = static_cast<size_t>(delay/samplInter);

  createHistList(_waveform->result().copy_sptr());

  Log::add(Log::INFO,"Processor '" + name() + "' is converting waveform '" +
           _waveform->name() + "' to a CFD Trace using delay '" + toString(delay) +
           "', Fraction '" + toString(_fraction) + "', Walk '" + toString(_walk) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp111::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram1DFloat& waveform
      (dynamic_cast<const Histogram1DFloat&>(_waveform->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&waveform.lock);

  const Histogram1DFloat::storage_t &Data(waveform.memory());
  Histogram1DFloat::storage_t &CFDTrace(result.memory());

  const size_t wLength(waveform.axis()[Histogram1DFloat::xAxis].size());
  /** set all points before the delay to 0 */
  fill(CFDTrace.begin(),CFDTrace.begin()+_delay,0);
  for (size_t i=_delay; i<wLength; ++i)
  {
    const float fx  = Data[i];            //the original Point at i
    const float fxd = Data[i-_delay];      //the delayed Point  at i
    const float fsx = -fx*_fraction + fxd; //the calculated CFPoint at i
    CFDTrace[i] = fsx - _walk;
  }

  result.nbrOfFills()=1;
}

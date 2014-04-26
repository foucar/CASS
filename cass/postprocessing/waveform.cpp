//Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file waveform.cpp file contains acqiris data retrieval postprocessor
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

using namespace cass;
using namespace ACQIRIS;
using std::runtime_error;
using std::minus;
using std::multiplies;
using std::cout;
using std::endl;
using std::invalid_argument;
using std::tr1::shared_ptr;
using std::tr1::bind;
using std::tr1::placeholders::_1;

//the last wavefrom postprocessor
pp110::pp110(const name_t &name)
  :PostProcessor(name)
{
  loadSettings(0);
}

void pp110::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
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
  Log::add(Log::INFO,"PostProcessor '" + name() + "' is showing channel '" +
           toString(_channel) + "' of acqiris '" + toString(_instrument) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp110::process(const CASSEvent &evt, HistogramBackend &res)
{
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));
  const Device *dev
      (dynamic_cast<const Device*>(evt.devices().find(CASSEvent::Acqiris)->second));
  Device::instruments_t::const_iterator instrIt (dev->instruments().find(_instrument));
  if (dev->instruments().end() == instrIt)
    throw runtime_error("pp110::process() '" + name() +
                        "': Data doesn't contain Instrument '"+toString(_instrument)
                        +"'");
  const Instrument &instr(instrIt->second);
  if (instr.channels().size() <= _channel)
    throw runtime_error("pp110::process() '" + name() + "Instrument '"+
                        toString(_instrument) + "' doesn't contain channel '" +
                        toString(_channel)+ "'");
  const Channel &channel (instr.channels()[_channel]);
  const waveform_t &waveform (channel.waveform());
  if (result.axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    throw invalid_argument("Postprocessor '" + name() +
                           "' incomming waveforms NbrSamples '" + toString(waveform.size()) +
                           "'. User set NbrSamples '" +
                           toString(result.axis()[HistogramBackend::xAxis].nbrBins()) +
                           "'");
  }
  if (!qFuzzyCompare(channel.sampleInterval(), _sampleInterval))
  {
    throw invalid_argument("Postprocessor '" + name() +
                           "' incomming waveforms SampleInterval '" + toString(channel.sampleInterval()) +
                           "'. User set SampleInterval '" + toString(_sampleInterval) + "'");
  }
  transform(waveform.begin(), waveform.end(),
            result.memory().begin(),
            bind(minus<float>(),
                 bind(multiplies<float>(),channel.gain(),_1),channel.offset()));
  result.nbrOfFills()=1;
}

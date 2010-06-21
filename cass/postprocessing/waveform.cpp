//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>

#include "waveform.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass.h"
#include "acqiris_device.h"
#include "convenience_functions.h"
#include "cass_settings.h"


//the last wavefrom postprocessor
cass::pp110::pp110(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp110::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _instrument = static_cast<Instruments>(settings.value("InstrumentId",8).toUInt());
  _channel    = settings.value("ChannelNbr",0).toUInt();
  if (!setupCondition())
    return;
  _result = new Histogram1DFloat(1,0,1);
  createHistList(2*cass::NbrOfWorkers);

  std::cout <<"PostProcessor "<<_key
      <<" is showing channel "<<_channel
      <<" of acqiris "<<_instrument
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp110::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  const Device *dev
      (dynamic_cast<const Device*>(evt.devices().find(CASSEvent::Acqiris)->second));
  Device::instruments_t::const_iterator instrIt (dev->instruments().find(_instrument));
  if (dev->instruments().end() == instrIt)
    throw std::runtime_error(QString("PostProcessor %1: Data doesn't contain Instrument %2")
                             .arg(_key.c_str())
                             .arg(_instrument).toStdString());
  const Instrument &instr(instrIt->second);
  if (instr.channels().size() <= _channel)
    throw std::runtime_error(QString("PostProcessor %1: Instrument %2 doesn't contain channel %3")
                             .arg(_key.c_str())
                             .arg(_instrument)
                             .arg(_channel).toStdString());
  const Channel &channel (instr.channels()[_channel]);
  const waveform_t &waveform (channel.waveform());
  QMutexLocker lock(&_mutex);
  if (_result->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
    dynamic_cast<Histogram1DFloat*>(_result)->resize(waveform.size(),0, waveform.size()*channel.sampleInterval());
  _result->lock.lockForWrite();
  std::transform(waveform.begin(),
                 waveform.end(),
                 dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
                 cass::Adc2Volts(channel.gain(),channel.offset()));
  _result->lock.unlock();
}



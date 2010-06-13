//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>

#include <QtCore/QSettings>

#include "waveform.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass.h"
#include "acqiris_device.h"


//the last wavefrom postprocessor
cass::pp110::pp110(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp110::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _instrument = static_cast<Instruments>(settings.value("InstrumentId",8).toUInt());
  _channel    = settings.value("ChannelNbr",0).toUInt();

  _result = new Histogram1DFloat(1,0,1);
  createHistList(2*cass::NbrOfWorkers);

  std::cout <<"PostProcessor "<<_key
      <<" is showing channel "<<_channel
      <<" of acqiris "<<_instrument
      <<std::endl;
}

void cass::pp110::process(const cass::CASSEvent &cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a pointer to the Acqiris device//
  const Device *dev
      (dynamic_cast<const Device*>(cassevent.devices().find(CASSEvent::Acqiris)->second));
  //retrieve a reference to the right instument//
  Device::instruments_t::const_iterator instrIt (dev->instruments().find(_instrument));
  //check if instrument exists//
  if (dev->instruments().end() == instrIt)
    throw std::runtime_error(QString("PostProcessor %1: Data doesn't contain Instrument %2")
                             .arg(_key.c_str())
                             .arg(_instrument).toStdString());
  const Instrument &instr(instrIt->second);
  //retrieve a reference to the right channel//
  if (instr.channels().size() <= _channel)
    throw std::runtime_error(QString("PostProcessor %1: Instrument %2 doesn't contain channel %3")
                             .arg(_key.c_str())
                             .arg(_instrument)
                             .arg(_channel).toStdString());
  const Channel &channel (instr.channels()[_channel]);
  //retrieve a reference to the waveform of the channel//
  const waveform_t &waveform (channel.waveform());
  //from here on only one thread should work at a time//
  QMutexLocker lock(&_mutex);
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of//
  //this channel//
  if (_result->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
    dynamic_cast<Histogram1DFloat*>(_result)->resize(waveform.size(),0, waveform.size()*channel.sampleInterval());
  //copy the waveform to our storage histogram and convert adc units to volts
  //while copying
  _result->lock.lockForWrite();
  std::transform(waveform.begin(),
                 waveform.end(),
                 dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
                 cass::ACQIRIS::Adc2Volts(channel.gain(),channel.offset()));
  _result->lock.unlock();
}

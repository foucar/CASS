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

using namespace cass;
using namespace ACQIRIS;
using std::runtime_error;
using std::minus;
using std::multiplies;
using std::cout;
using std::endl;
using std::tr1::bind;
using std::tr1::placeholders::_1;

//the last wavefrom postprocessor
pp110::pp110(PostProcessors &pp, const PostProcessors::key_t &key)
  :PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void pp110::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _instrument = static_cast<Instruments>(settings.value("InstrumentId",8).toUInt());
  _channel    = settings.value("ChannelNbr",0).toUInt();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram1DFloat(1,0,1,"Time [s]");
  createHistList(2*NbrOfWorkers);
  cout<< endl <<"PostProcessor '"<<_key
      <<"' is showing channel '"<<_channel
      <<"' of acqiris '"<<_instrument
      <<"'. Condition is '"<<_condition->key()<<"'"
   <<endl;
}

void pp110::process(const CASSEvent &evt)
{
  const Device *dev
      (dynamic_cast<const Device*>(evt.devices().find(CASSEvent::Acqiris)->second));
  Device::instruments_t::const_iterator instrIt (dev->instruments().find(_instrument));
  if (dev->instruments().end() == instrIt)
    throw runtime_error(QString("PostProcessor %1: Data doesn't contain Instrument %2")
                        .arg(_key.c_str())
                        .arg(_instrument).toStdString());
  const Instrument &instr(instrIt->second);
  if (instr.channels().size() <= _channel)
    throw runtime_error(QString("PostProcessor %1: Instrument %2 doesn't contain channel %3")
                        .arg(_key.c_str())
                        .arg(_instrument)
                        .arg(_channel).toStdString());
  const Channel &channel (instr.channels()[_channel]);
  const waveform_t &waveform (channel.waveform());
  _result->lock.lockForWrite();
  if (_result->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    for (histogramList_t::iterator it(_histList.begin()); it != _histList.end(); ++it)
    {
      dynamic_cast<Histogram1DFloat*>(it->second)->resize(waveform.size(),
                                                          0,
                                                          waveform.size()*channel.sampleInterval());
    }
    PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
    PostProcessors::keyList_t::iterator it (dependands.begin());
    for (; it != dependands.end(); ++it)
      _pp.getPostProcessor(*it).histogramsChanged(_result);
  }
  transform(waveform.begin(), waveform.end(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            bind<float>(minus<float>(),
                        bind(multiplies<float>(),channel.gain(),_1),channel.offset()));
  _result->nbrOfFills()=1;
  _result->lock.unlock();
}

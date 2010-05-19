//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>

#include <QtCore/QSettings>

#include "waveform.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"


//the last wavefrom postprocessor
cass::pp4::pp4(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _waveform(0)
{
  loadSettings(0);
}

cass::pp4::~pp4()
{
  _pp.histograms_delete(_id);
  _waveform=0;
}

void cass::pp4::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  /** @todo find a way to resize the histogram without deleting it */
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());

  _instrument = static_cast<Instruments>(settings.value("InstrumentId",8).toUInt());
  _channel    = settings.value("ChannelNbr",0).toUInt();

  _pp.histograms_delete(_key);
  _waveform = new Histogram1DFloat(1,0,1);
  _pp.histograms_replace(_key,_waveform);

  std::cout <<"PostProcessor "<<_key
      <<" is showing channel "<<_channel
      <<" of acqiris "<<_instrument
      <<std::endl;
}

void cass::pp4::operator()(const cass::CASSEvent &cassevent)
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
                             .arg(_instrument));
  const Instrument &instr(instrIt->second);
  //retrieve a reference to the right channel//
  if (instr.channels().size() <= _channel)
    throw std::runtime_error(QString("PostProcessor %1: Instrument %2 doesn't contain channel %3")
                             .arg(_key.c_str())
                             .arg(_instrument)
                             .arg(_channel));
  const Channel &channel (instr.channels()[_channel]);
  //retrieve a reference to the waveform of the channel//
  const waveform_t &waveform (channel.waveform());
  //from here on only one thread should work at a time//
  QMutexLocker lock(&_mutex);
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of//
  //this channel//
  if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
    _waveform->resize(waveform.size(),0, waveform.size()*channel.sampleInterval());
  //copy the waveform to our storage histogram and convert adc units to volts
  //while copying
  _waveform->lock.lockForWrite();
  std::transform(waveform.begin(),
                 waveform.end(),
                 _waveform->memory().begin(),
                 cass::ACQIRIS::Adc2Volts(channel.gain(),channel.offset()));
  _waveform->lock.unlock();
}














//the average waveform creator//
cass::pp500::pp500(cass::PostProcessors &ppc, cass::PostProcessors::id_t id)
  :cass::PostprocessorBackend(ppc,id),
  _waveform(0)
{
  using namespace cass::ACQIRIS;
  switch (_id)
  {
  case PostProcessors::CampChannel00AveragedWaveform:
    _channel=0;_instrument=Camp1;_idSingle=PostProcessors::CampChannel00LastWaveform;break;
  case PostProcessors::CampChannel01AveragedWaveform:
    _channel=1;_instrument=Camp1;_idSingle=PostProcessors::CampChannel01LastWaveform;break;
  case PostProcessors::CampChannel02AveragedWaveform:
    _channel=2;_instrument=Camp1;_idSingle=PostProcessors::CampChannel02LastWaveform;break;
  case PostProcessors::CampChannel03AveragedWaveform:
    _channel=3;_instrument=Camp1;_idSingle=PostProcessors::CampChannel03LastWaveform;break;
  case PostProcessors::CampChannel04AveragedWaveform:
    _channel=4;_instrument=Camp1;_idSingle=PostProcessors::CampChannel04LastWaveform;break;
  case PostProcessors::CampChannel05AveragedWaveform:
    _channel=5;_instrument=Camp1;_idSingle=PostProcessors::CampChannel05LastWaveform;break;
  case PostProcessors::CampChannel06AveragedWaveform:
    _channel=6;_instrument=Camp1;_idSingle=PostProcessors::CampChannel06LastWaveform;break;
  case PostProcessors::CampChannel07AveragedWaveform:
    _channel=7;_instrument=Camp1;_idSingle=PostProcessors::CampChannel07LastWaveform;break;
  case PostProcessors::CampChannel08AveragedWaveform:
    _channel=8;_instrument=Camp1;_idSingle=PostProcessors::CampChannel08LastWaveform;break;
  case PostProcessors::CampChannel09AveragedWaveform:
    _channel=9;_instrument=Camp1;_idSingle=PostProcessors::CampChannel09LastWaveform;break;
  case PostProcessors::CampChannel10AveragedWaveform:
    _channel=10;_instrument=Camp1;_idSingle=PostProcessors::CampChannel10LastWaveform;break;
  case PostProcessors::CampChannel11AveragedWaveform:
    _channel=11;_instrument=Camp1;_idSingle=PostProcessors::CampChannel11LastWaveform;break;
  case PostProcessors::CampChannel12AveragedWaveform:
    _channel=12;_instrument=Camp1;_idSingle=PostProcessors::CampChannel12LastWaveform;break;
  case PostProcessors::CampChannel13AveragedWaveform:
    _channel=13;_instrument=Camp1;_idSingle=PostProcessors::CampChannel13LastWaveform;break;
  case PostProcessors::CampChannel14AveragedWaveform:
    _channel=14;_instrument=Camp1;_idSingle=PostProcessors::CampChannel14LastWaveform;break;
  case PostProcessors::CampChannel15AveragedWaveform:
    _channel=15;_instrument=Camp1;_idSingle=PostProcessors::CampChannel15LastWaveform;break;
  case PostProcessors::CampChannel16AveragedWaveform:
    _channel=16;_instrument=Camp1;_idSingle=PostProcessors::CampChannel16LastWaveform;break;
  case PostProcessors::CampChannel17AveragedWaveform:
    _channel=17;_instrument=Camp1;_idSingle=PostProcessors::CampChannel17LastWaveform;break;
  case PostProcessors::CampChannel18AveragedWaveform:
    _channel=18;_instrument=Camp1;_idSingle=PostProcessors::CampChannel18LastWaveform;break;
  case PostProcessors::CampChannel19AveragedWaveform:
    _channel=19;_instrument=Camp1;_idSingle=PostProcessors::CampChannel19LastWaveform;break;
  case PostProcessors::ITofChannel00AveragedWaveform:
    _channel=0;_instrument=Camp2;_idSingle=PostProcessors::ITofChannel00LastWaveform;break;
  case PostProcessors::ITofChannel01AveragedWaveform:
    _channel=1;_instrument=Camp2;_idSingle=PostProcessors::ITofChannel01LastWaveform;break;
  case PostProcessors::ITofChannel02AveragedWaveform:
    _channel=2;_instrument=Camp2;_idSingle=PostProcessors::ITofChannel02LastWaveform;break;
  case PostProcessors::ITofChannel03AveragedWaveform:
    _channel=3;_instrument=Camp2;_idSingle=PostProcessors::ITofChannel03LastWaveform;break;

  default: throw std::invalid_argument(QString("postprocessor %1 is not for averaging waveforms").arg(_id).toStdString());
  }
  loadSettings(0);
}

cass::pp500::~pp500()
{
  _pp.histograms_delete(_id);
  _waveform=0;
}


void cass::pp500::operator ()(const cass::CASSEvent &)
{
  //get the histogram with the single waveform//
  Histogram1DFloat *singleWaveform (dynamic_cast<Histogram1DFloat *>(_pp.histograms_checkout().find(_idSingle)->second));
  _pp.histograms_release();

  //from here on only one thread should work at a time//
  QMutexLocker lock(&_mutex);
  //check the whether the range is the same//
  if (!singleWaveform) return;
  singleWaveform->lock.lockForRead();
  if (!_waveform || (_waveform && _waveform->axis()[HistogramBackend::xAxis].nbrBins() !=
                     singleWaveform->axis()[HistogramBackend::xAxis].nbrBins()))
  {
    _pp.histograms_delete(_id);
    _waveform = new Histogram1DFloat(singleWaveform->axis()[HistogramBackend::xAxis].nbrBins(),
                                     singleWaveform->axis()[HistogramBackend::xAxis].lowerLimit(),
                                     singleWaveform->axis()[HistogramBackend::xAxis].upperLimit());
    _pp.histograms_replace(_id,_waveform);
  }
  //choose which kind of average we want to have//
  //if alpha is 1 then we want a cummulative average,//
  //so alpha needs to be 1/nbrofFills+1//
  //otherwise we just use the calculated alpha//
  _waveform->lock.lockForRead();
  const float alpha = (std::abs(_alpha-1.)<1e-15) ?
                      1./(_waveform->nbrOfFills()+1.) :
                      _alpha;
  _waveform->lock.unlock();

  _waveform->lock.lockForWrite();
  //average the waveform and put the result in the averaged waveform//
  std::transform(singleWaveform->memory().begin(),//start of src
                 singleWaveform->memory().end(),  //one beyond stop of src
                 _waveform->memory().begin(),     //start of second src
                 _waveform->memory().begin(),     //start of result
                 Average(alpha));                 //the binary operator
  //tell the histogram that we just filled it//
  singleWaveform->lock.unlock();
  ++_waveform->nbrOfFills();
  _waveform->lock.unlock();
}

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
cass::pp4::pp4(cass::PostProcessors &pp, cass::PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
  _waveform(0)
{
  using namespace cass::ACQIRIS;
  switch(_id)
  {
  case PostProcessors::CampChannel00LastWaveform: _channel=0;_instrument=Camp1;break;
  case PostProcessors::CampChannel01LastWaveform: _channel=1;_instrument=Camp1;break;
  case PostProcessors::CampChannel02LastWaveform: _channel=2;_instrument=Camp1;break;
  case PostProcessors::CampChannel03LastWaveform: _channel=3;_instrument=Camp1;break;
  case PostProcessors::CampChannel04LastWaveform: _channel=4;_instrument=Camp1;break;
  case PostProcessors::CampChannel05LastWaveform: _channel=5;_instrument=Camp1;break;
  case PostProcessors::CampChannel06LastWaveform: _channel=6;_instrument=Camp1;break;
  case PostProcessors::CampChannel07LastWaveform: _channel=7;_instrument=Camp1;break;
  case PostProcessors::CampChannel08LastWaveform: _channel=8;_instrument=Camp1;break;
  case PostProcessors::CampChannel09LastWaveform: _channel=9;_instrument=Camp1;break;
  case PostProcessors::CampChannel10LastWaveform: _channel=10;_instrument=Camp1;break;
  case PostProcessors::CampChannel11LastWaveform: _channel=11;_instrument=Camp1;break;
  case PostProcessors::CampChannel12LastWaveform: _channel=12;_instrument=Camp1;break;
  case PostProcessors::CampChannel13LastWaveform: _channel=13;_instrument=Camp1;break;
  case PostProcessors::CampChannel14LastWaveform: _channel=14;_instrument=Camp1;break;
  case PostProcessors::CampChannel15LastWaveform: _channel=15;_instrument=Camp1;break;
  case PostProcessors::CampChannel16LastWaveform: _channel=16;_instrument=Camp1;break;
  case PostProcessors::CampChannel17LastWaveform: _channel=17;_instrument=Camp1;break;
  case PostProcessors::CampChannel18LastWaveform: _channel=18;_instrument=Camp1;break;
  case PostProcessors::CampChannel19LastWaveform: _channel=19;_instrument=Camp1;break;
  case PostProcessors::ITofChannel00LastWaveform: _channel=0;_instrument=Camp2;break;
  case PostProcessors::ITofChannel01LastWaveform: _channel=1;_instrument=Camp2;break;
  case PostProcessors::ITofChannel02LastWaveform: _channel=2;_instrument=Camp2;break;
  case PostProcessors::ITofChannel03LastWaveform: _channel=3;_instrument=Camp2;break;
  default: throw std::invalid_argument(QString("postprocessor %1 is not for the last waveform").arg(_id).toStdString());
  }
  std::cout<<std::endl<< "PostProcessor_"<<_id<< " will show the last wavform of Channel "<< _channel
      <<" of Instrument "<<_instrument<<std::endl;
}

cass::pp4::~pp4()
{
  _pp.histograms_delete(_id);
  _waveform=0;
}

void cass::pp4::operator()(const cass::CASSEvent &cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a pointer to the Acqiris device//
  const Device *dev =
      dynamic_cast<const Device*>(cassevent.devices().find(CASSEvent::Acqiris)->second);
  //retrieve a reference to the right instument//
  Device::instruments_t::const_iterator instrI = dev->instruments().find(_instrument);
  //check if instrument exists//
  if (instrI == dev->instruments().end())
  {
    std::cerr << "did not find the requested Acqiris instrument: "<<_instrument
        << " maybe the Acqiris converter is not active"<<std::endl;
    return;
  }
  const Instrument &instr = instrI->second;
  //retrieve a reference to the right channel//
  if (instr.channels().size() <= _channel)
  {
    std::cerr << "In the current configuration now instrument "<<_instrument
        << " does not have channel "<< _channel
        << ". Check the configuration"<<std::endl;
    return;
  }
  const Channel &channel = instr.channels()[_channel];
  //retrieve a reference to the waveform of the channel//
  const waveform_t &waveform = channel.waveform();
  //from here on only one thread should work at a time//
  QMutexLocker lock(&_mutex);
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of//
  //this channel//
  if(!_waveform)
  {
    _waveform =
        new Histogram1DFloat(waveform.size(),
                             0,
                             channel.fullscale()*channel.sampleInterval());
    _pp.histograms_replace(_id,_waveform);
  }
  else if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    _pp.histograms_delete(_id);
    _waveform = new Histogram1DFloat(waveform.size(),
                                     0,
                                     channel.fullscale()*channel.sampleInterval());
    _pp.histograms_replace(_id,_waveform);
  }
  //copy the waveform to our storage histogram
  _waveform->lock.lockForWrite();
//  std::copy(waveform.begin(),waveform.end(),_waveform->memory().begin());
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

void cass::pp500::loadSettings(size_t)
{
  QSettings parameter;
  parameter.beginGroup("PostProcessor");
  parameter.beginGroup(QString("p") + QString::number(_id));
  //load the nbr of averages, and calculate the alpha from it//
  uint32_t N = parameter.value("NumberOfAverages",1).toUInt();
  _alpha = static_cast<float>(2./(N+1));
  std::cout <<"PostProcessor_"<<_id<<" is averaging over "<<N<<" events."
      <<" alpha is :"<<_alpha
      <<" Input is PostProcessor_"<<_idSingle
      <<std::endl;
}

void cass::pp500::operator ()(const cass::CASSEvent &)
{
  //get the histogram with the single waveform//
  Histogram1DFloat *singleWaveform (dynamic_cast<Histogram1DFloat *>(_pp.histograms_checkout().find(_idSingle)->second));
  _pp.histograms_release();

  //from here on only one thread should work at a time//
  QMutexLocker lock(&_mutex);
  //check the whether the range is the same//
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

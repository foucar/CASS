//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QSettings>

#include "waveform.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"


//the last wavefrom copier
cass::pp4::pp4(cass::PostProcessors::histograms_t &hist, cass::PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _channel(300),
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
  default: throw std::invalid_argument("channel does not exist in argument list");
  }
}

cass::pp4::~pp4()
{
  delete _waveform;
  _waveform=0;
}

void cass::pp4::operator()(const cass::CASSEvent &cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a pointer to the Acqiris device//
  const Device *dev =
      dynamic_cast<const Device*>(cassevent.devices().find(CASSEvent::Acqiris)->second);
  //retrieve a reference to the right instument//
  const Instrument &instr = dev->instruments().find(_instrument)->second;
  //retrieve a reference to the right channel//
  const Channel &channel =instr.channels()[_channel];
  //retrieve a reference to the waveform of the channel//
  const Channel::waveform_t &waveform = channel.waveform();
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of //
  //this channel//
  if(!_waveform)
  {
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
    _histograms[_id] = _waveform;
  }
  else if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    delete _waveform;
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
    _histograms[_id] = _waveform;

  }
  //copy the waveform to our storage histogram//
  std::copy(waveform.begin(),waveform.end(),_waveform->memory().begin());
}





namespace cass
{
  /*! binary operator for averaging
    capable of performing a Cumulative moving average and
    a Exponential moving average.
   @see http://en.wikipedia.org/wiki/Moving_average
  @author Lutz Foucar
  */
  class Average
  {
  public:
    /** constructor initializing the \f$\alpha\f$ value*/
    explicit Average(float alpha)
      :_alpha(alpha)
    {}
    /** the operator calculates the average using the function
      \f$Y_n = \alpha(y-Y_{n-1})\f$
      where when \alpha is equal to N it is a cumulative moving average,
      otherwise it will be a exponential moving average.*/
    float operator()(float Average_Nm1, float currentValue)
    {
      return Average_Nm1 + _alpha*(currentValue - Average_Nm1);
    }
  protected:
    /** \f$\alpha\f$ for the average calculation \f$ */
    float _alpha;
  };
}

//the average waveform creator//
cass::pp500::pp500(cass::PostProcessors::histograms_t &hist, cass::PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id)
{
  using namespace cass::ACQIRIS;
  switch (_id)
  {
  case PostProcessors::CampChannel00AveragedWaveform: _channel=0;_instrument=Camp1;break;
  case PostProcessors::CampChannel01AveragedWaveform: _channel=1;_instrument=Camp1;break;
  case PostProcessors::CampChannel02AveragedWaveform: _channel=2;_instrument=Camp1;break;
  case PostProcessors::CampChannel03AveragedWaveform: _channel=3;_instrument=Camp1;break;
  case PostProcessors::CampChannel04AveragedWaveform: _channel=4;_instrument=Camp1;break;
  case PostProcessors::CampChannel05AveragedWaveform: _channel=5;_instrument=Camp1;break;
  case PostProcessors::CampChannel06AveragedWaveform: _channel=6;_instrument=Camp1;break;
  case PostProcessors::CampChannel07AveragedWaveform: _channel=7;_instrument=Camp1;break;
  case PostProcessors::CampChannel08AveragedWaveform: _channel=8;_instrument=Camp1;break;
  case PostProcessors::CampChannel09AveragedWaveform: _channel=9;_instrument=Camp1;break;
  case PostProcessors::CampChannel10AveragedWaveform: _channel=10;_instrument=Camp1;break;
  case PostProcessors::CampChannel11AveragedWaveform: _channel=11;_instrument=Camp1;break;
  case PostProcessors::CampChannel12AveragedWaveform: _channel=12;_instrument=Camp1;break;
  case PostProcessors::CampChannel13AveragedWaveform: _channel=13;_instrument=Camp1;break;
  case PostProcessors::CampChannel14AveragedWaveform: _channel=14;_instrument=Camp1;break;
  case PostProcessors::CampChannel15AveragedWaveform: _channel=15;_instrument=Camp1;break;
  case PostProcessors::CampChannel16AveragedWaveform: _channel=16;_instrument=Camp1;break;
  case PostProcessors::CampChannel17AveragedWaveform: _channel=17;_instrument=Camp1;break;
  case PostProcessors::CampChannel18AveragedWaveform: _channel=18;_instrument=Camp1;break;
  case PostProcessors::CampChannel19AveragedWaveform: _channel=19;_instrument=Camp1;break;
  default: throw std::invalid_argument("the requested postprocessor is not handled by this one");
  }
}

cass::pp500::~pp500()
{
  delete _waveform;
  _waveform=0;
}

void cass::pp500::loadParameters(size_t)
{
  QSettings parameter;
  parameter.beginGroup("postprocessors");
  parameter.beginGroup(QString("processor_") + QString::number(_id));
  //load the nbr of averages, and calculate the alpha from it//
  uint32_t N = parameter.value("NumberOfAverages",1).toUInt();
  _alpha = static_cast<float>(1./N);
}

void cass::pp500::operator ()(const cass::CASSEvent & cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a pointer to the Acqiris device//
  const Device *dev =
      dynamic_cast<const Device*>(cassevent.devices().find(CASSEvent::Acqiris)->second);
  //retrieve a reference to the right instument//
  const Instrument &instr = dev->instruments().find(_instrument)->second;
  //retrieve a reference to the right channel//
  const Channel &channel =instr.channels()[_channel];
  //retrieve a reference to the waveform of the channel//
  const Channel::waveform_t &waveform = channel.waveform();
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of //
  //this channel//
  if(!_waveform)
  {
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
    _histograms[_id] = _waveform;
  }
  else if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    delete _waveform;
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
    _histograms[_id] = _waveform;
  }
  //choose which kind of average we want to have//
  //if alpha is 1 then we want a cummulative average,//
  //so alpha needs to be 1/nbrofFills+1//
  //otherwise we just use the calculated alpha//
  const float alpha = (std::abs(_alpha-1.)<1e-15)?
                      1./(_waveform->nbrOfFills()+1.)
                        : _alpha;

  //average the waveform and put the result in the averaged waveform//
  std::transform(waveform.begin(),waveform.end(),
                 _waveform->memory().begin(),
                 _waveform->memory().begin(),
                 Average(alpha));
  //tell the histogram that we just filled it//
  ++_waveform->nbrOfFills();
}

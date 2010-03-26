//Copyright (C) 2010 lmf

#include "waveform_postprocessor.h"
#include "cass_event.h"
#include "acqiris_device.h"

//the last wavefrom copier
cass::LastWaveform::LastWaveform(cass::PostProcessors::histograms_t &hist, cass::PostProcessors::id_t id)
  :cass::PostProcessorBackend(hist,id),
  _waveform(0),
  _channel(300)
{
  switch(_id)
  {
  case PostProcessors::CampChannel00LastWaveform: _channel=0;break;
  case PostProcessors::CampChannel01LastWaveform: _channel=1;break;
  case PostProcessors::CampChannel02LastWaveform: _channel=2;break;
  case PostProcessors::CampChannel03LastWaveform: _channel=3;break;
  case PostProcessors::CampChannel04LastWaveform: _channel=4;break;
  case PostProcessors::CampChannel05LastWaveform: _channel=5;break;
  case PostProcessors::CampChannel06LastWaveform: _channel=6;break;
  case PostProcessors::CampChannel07LastWaveform: _channel=7;break;
  case PostProcessors::CampChannel08LastWaveform: _channel=8;break;
  case PostProcessors::CampChannel09LastWaveform: _channel=9;break;
  case PostProcessors::CampChannel10LastWaveform: _channel=10;break;
  case PostProcessors::CampChannel11LastWaveform: _channel=11;break;
  case PostProcessors::CampChannel12LastWaveform: _channel=12;break;
  case PostProcessors::CampChannel13LastWaveform: _channel=13;break;
  case PostProcessors::CampChannel14LastWaveform: _channel=14;break;
  case PostProcessors::CampChannel15LastWaveform: _channel=15;break;
  case PostProcessors::CampChannel16LastWaveform: _channel=16;break;
  case PostProcessors::CampChannel17LastWaveform: _channel=17;break;
  case PostProcessors::CampChannel18LastWaveform: _channel=18;break;
  case PostProcessors::CampChannel19LastWaveform: _channel=19;break;
  default: throw std::invalid_argument("channel does not exist in argument list");
  }
}

cass::LastWaveform::~LastWaveform()
{ 
  delete _waveform;
  waveform=0;
}

void cass::LastWaveform::operator()(const CASSEvent &cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a reference to the wavefrom of the wanted channel//
  const AcqirisDevice *dev =
      dynamic_cast<const AcqirisDevice*>(cassevent->devices()[CASSEvent::Acqiris]);
  const Channel &channel = dev->channels(_channel);
  const Channel::waveform_t &waveform = channel.waveform();
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of //
  //this channel//
  if(!_waveform)
    _waveform = new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
  else if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    delete waveform;
    _waveform = new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
  }
  //copy the waveform to our storage histogram//
  std::copy(waveform.begin(),waveform.end(),_waveform->memory().begin());
}





namespace cass
{
  //binary operator for averaging//
  class Average
  {
    explicit Average(float alpha)
      :_alpha(alpha)
    {}
    float operator()(float Average_Nm1, float currentValue)
    {
      return Average_Nm1 + _alpha*(currentValue - Average_Nm1);
    }
  protected:
    float _alpha;
  };
}

//the average waveform creator//
cass::AverageWaveform(cass::PostProcessors::histograms_t &hist, cass::PostProcessors::id_t id)
  :cass::PostProcessorBackend(hist,id)
{
  switch (_id)
  {
  case PostProcessors::CampChannel00AveragedWaveform: _channel=0;break;
  case PostProcessors::CampChannel01AveragedWaveform: _channel=1;break;
  case PostProcessors::CampChannel02AveragedWaveform: _channel=2;break;
  case PostProcessors::CampChannel03AveragedWaveform: _channel=3;break;
  case PostProcessors::CampChannel04AveragedWaveform: _channel=4;break;
  case PostProcessors::CampChannel05AveragedWaveform: _channel=5;break;
  case PostProcessors::CampChannel06AveragedWaveform: _channel=6;break;
  case PostProcessors::CampChannel07AveragedWaveform: _channel=7;break;
  case PostProcessors::CampChannel08AveragedWaveform: _channel=8;break;
  case PostProcessors::CampChannel09AveragedWaveform: _channel=9;break;
  case PostProcessors::CampChannel10AveragedWaveform: _channel=10;break;
  case PostProcessors::CampChannel11AveragedWaveform: _channel=11;break;
  case PostProcessors::CampChannel12AveragedWaveform: _channel=12;break;
  case PostProcessors::CampChannel13AveragedWaveform: _channel=13;break;
  case PostProcessors::CampChannel14AveragedWaveform: _channel=14;break;
  case PostProcessors::CampChannel15AveragedWaveform: _channel=15;break;
  case PostProcessors::CampChannel16AveragedWaveform: _channel=16;break;
  case PostProcessors::CampChannel17AveragedWaveform: _channel=17;break;
  case PostProcessors::CampChannel18AveragedWaveform: _channel=18;break;
  case PostProcessors::CampChannel19AveragedWaveform: _channel=19;break;
  default: throw std::invalid_argument("channel does not exist in argument list");
  }
}

cass::AverageWavefrom::~AverageWavefrom()
{ 
  delete _waveform;
  waveform=0;
}

cass::AverageWavefrom::loadParameters()
{
  QSettings parameter;
  parameter.beginGroup("postprocessors");
  parameter.beginGroup(QString("processor_") + QString::number(_id));
  //load the nbr of averages, and calculate the alpha from it//
  uint32_t N = parameter.value("NumberOfAverages",1).toUInt();
  _alpha = static_cast<float>(1./N);
}

cass::AverageWavefrom::operator ()(const cass::CASSEvent & cassevent)
{
  using namespace cass::ACQIRIS;
  //retrieve a reference to the wavefrom of the wanted channel//
  const AcqirisDevice *dev =
      dynamic_cast<const AcqirisDevice*>(cassevent->devices()[CASSEvent::Acqiris]);
  const Channel &channel = dev->channels(_channel);
  const Channel::waveform_t &waveform = channel.waveform();
  //check wether the wavefrom histogram has been created//
  //and is still valid for the now incomming wavefrom of //
  //this channel//
  if(!_waveform)
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
  else if (_waveform->axis()[HistogramBackend::xAxis].nbrBins() != waveform.size())
  {
    delete waveform;
    _waveform =
        new Histogram1DFloat(waveform.size(),0,channel.fullscale()*channel.sampleInterval());
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

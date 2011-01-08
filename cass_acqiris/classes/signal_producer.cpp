//Copyright (C) 2010 Lutz Foucar

#include "signal_producer.h"
#include "cass_settings.h"

#include "signal_extractor.h"

using namespace cass::ACQIRIS;

void SignalProducer::loadSettings(CASSSettings &p)
{
  delete _signalextractor;
  SignalExtractorType analyzerType
      (static_cast<SignalExtractorType>(p.value("SignalExtactorType",com16).toInt()));
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(p);
//  _grLow        = p->value("LowerGoodTimeRangeLimit",0.).toDouble();
//  _grHigh       = p->value("UpperGoodTimeRangeLimit",20000.).toDouble();
}

SignalProducer::signals_t& SignalProducer::output()
{
  bool newEventAssociated (_newEventAssociated);
  _newEventAssociated = false;
  return (newEventAssociated)? (*_signalextractor)(_signals):_signals;
}

void SignalProducer::associate(const CASSEvent &evt)
{
 _newEventAssociated = true;
 _signals.clear();
 _signalextractor->associate(evt);
}

double SignalProducer::firstGood() const
{
//  //if this is called for the new event for the first time, then evaluate//
//  if(_isNewEvent)
//  {
//    //find first occurence of peak that is in the given timerange//
//    peaks_t::const_iterator it =
//        std::find_if(_peaks.begin(),_peaks.end(),
//                     PeakInRange(_grLow,_grHigh));
//    //if it is not there retrun 0, otherwise the time of the found peak//
//    _goodHit = (it==_peaks.end())? 0. : it->time();
//
//    _isNewEvent = false;
//  }
  return _goodHit;
}

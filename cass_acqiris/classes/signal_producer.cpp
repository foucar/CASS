//Copyright (C) 2010 Lutz Foucar

#include "signal_producer.h"
#include "cass_settings.h"

#include "signal_extractor.h"

using namespace cass::ACQIRIS;

void SignalProducer::loadSettings(CASSSettings &p, const char * signalname)
{
  VERBOSEOUT(std::cerr<<"Signal load parameters:  load signal parameters for signal \""
             <<signalname
             <<"\" of  "<< p->group().toStdString()
             <<std::endl);
  p.beginGroup(signalname);
  delete _signalextractor;
  SignalExtractorType analyzerType
      (static_cast<SignalExtractorType>(p.value("SignalExtactorType",com16).toInt()));
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(p);
  p.endGroup();



//  _instrument   = static_cast<Instruments>(p->value("AcqirisInstrument",Camp1).toInt());
//  VERBOSEOUT(std::cerr <<"Signal load parameters: Instrument "<<_instrument<<std::endl);
//  _chNbr        = p->value("ChannelNumber",0).toInt();
//  VERBOSEOUT(std::cerr <<"Signal load parameters: chNbr "<<_chNbr<<std::endl);
//  _trLow        = p->value("LowerTimeRangeLimit",0.).toDouble();
//  _trHigh       = p->value("UpperTimeRangeLimit",20000.).toDouble();
//  _grLow        = p->value("LowerGoodTimeRangeLimit",0.).toDouble();
//  _grHigh       = p->value("UpperGoodTimeRangeLimit",20000.).toDouble();
//  _polarity     = static_cast<Polarity>(p->value("Polarity",Negative).toInt());
//  _threshold    = p->value("Threshold",0.05).toDouble();
//  VERBOSEOUT(std::cerr <<"Signal load parameters: Threshold "<<_threshold<<std::endl);
//  _delay        = p->value("Delay",5).toInt();
//  _fraction     = p->value("Fraction",0.6).toDouble();
//  _walk         = p->value("Walk",0.).toDouble();
//  VERBOSEOUT(std::cerr <<"Signal load parameters: ana type "<<_analyzerType<<" should be "<<com16<<std::endl);
//  VERBOSEOUT(std::cerr<<"Signal load parameters: done loading"<<std::endl);
//  p->endGroup();
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

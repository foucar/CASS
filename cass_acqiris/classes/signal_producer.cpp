//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>

#include "signal_producer.h"
#include "cass_settings.h"

#include "signal_extractor.h"

using namespace cass::ACQIRIS;

namespace cass
{
  namespace ACQIRIS
  {
    /** functor returning true if signal is in requested range
     *
     * @author Lutz Foucar
     */
    class isInTimeRange : std::unary_function<SignalProducer::signal_t,bool>
    {
    public:
      /** constructor intializing the range*/
      isInTimeRange(const std::pair<double,double>& range)
        :_range(range)
      {}

      bool operator()(const SignalProducer::signal_t &sig)const
      {
        return (sig["time"] >_range.first && sig["time"] <= _range.second);
      }

    private:
      /** the range*/
      std::pair<double,double> _range;
    };
  }
}

SignalProducer::SignalProducer(const SignalProducer& rhs)
  :_goodHit(rhs._goodHit),
   _signalextractor(rhs._signalextractor),
   _signals(rhs._signals),
   _newEventAssociated(rhs._newEventAssociated)
{
}

SignalProducer::~SignalProducer()
{
//  delete _signalextractor;
}

void SignalProducer::loadSettings(CASSSettings &s)
{
//  delete _signalextractor;
  SignalExtractorType analyzerType
      (static_cast<SignalExtractorType>(s.value("SignalExtractionMethod",com16).toInt()));
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(s);
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

double SignalProducer::firstGood(const std::pair<double,double>& range)
{
  if(_newEventAssociated)
  {
    try
    {
      signals_t sigs (output());
      _goodHit = (*std::find_if(sigs.begin(),sigs.end(), isInTimeRange(range)))["time"];
    }
    /** @warning this relies on that when accessing a thing from end() iterator
     *           an exception is thrown. Needs checking
     */
    catch(const std::range_error&)
    {
      _goodHit=0;
    }
  }
  return _goodHit;
}

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
    /** functor retruning true if signal is in requested range
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

void SignalProducer::loadSettings(CASSSettings &p)
{
  delete _signalextractor;
  SignalExtractorType analyzerType
      (static_cast<SignalExtractorType>(p.value("SignalExtractionMethod",com16).toInt()));
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(p);
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
    catch(const std::range_error&)
    {}
  }
  return _goodHit;
}

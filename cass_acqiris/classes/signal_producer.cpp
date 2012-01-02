//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_producer.cpp file contains the classes that describe how to
 *                           analyze the waveform and stores the result.
 *
 * @author Lutz Foucar
 */
#include <stdexcept>
#include <iostream>

#include "signal_producer.h"
#include "cass_settings.h"

#include "signal_extractor.h"

using namespace cass::ACQIRIS;
using namespace std;

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
      /** the range */
      std::pair<double,double> _range;
    };
  }
}

void SignalProducer::loadSettings(CASSSettings &s)
{
  SignalExtractorType analyzerType
      (static_cast<SignalExtractorType>(s.value("SignalExtractionMethod",com16).toInt()));
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(s);
  _range = make_pair(s.value("GoodRangeLow",0).toDouble(),
                     s.value("GoodRangeHigh",0).toDouble());
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
  _goodHitExtracted = false;
  _signals.clear();
  _goodHit = 0;
  _signalextractor->associate(evt);
}

double SignalProducer::firstGood(const std::pair<double,double>& range)
{
  if(!_goodHitExtracted)
  {
    _goodHitExtracted = true;
    signals_t &sigs (output());
    signals_t::iterator sigIt(find_if(sigs.begin(),sigs.end(),isInTimeRange(range)));
    _goodHit = (sigIt != sigs.end())? (*sigIt)["time"] : 0;
  }
  return _goodHit;
}

double SignalProducer::firstGood()
{
	if(!_goodHitExtracted)
  {
    _goodHitExtracted = true;
    signals_t &sigs (output());
    signals_t::iterator sigIt(find_if(sigs.begin(),sigs.end(), isInTimeRange(_range)));
    _goodHit = (sigIt != sigs.end())? (*sigIt)["time"] : 0;
  }
  return _goodHit;
}

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
     * @todo replace this by bind..
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
// 	std::cout << s.group().toStdString()<< " signextractmethod "<<analyzerType<<std::endl;
  _signalextractor = SignalExtractor::instance(analyzerType);
  _signalextractor->loadSettings(s);
  _range = make_pair(s.value("GoodRangeLow",0).toDouble(),
                     s.value("GoodRangeHigh",0).toDouble());
}

SignalProducer::signals_t& SignalProducer::output()
{
  bool newEventAssociated (_newEventAssociated);
  _newEventAssociated = false;
//  std::cout << newEventAssociated<<std::endl;
  return (newEventAssociated)? (*_signalextractor)(_signals):_signals;
}

void SignalProducer::associate(const CASSEvent &evt)
{
//  std::cout <<"      SigProd 1"<<std::endl;
  _newEventAssociated = true;
  _goodHitExtracted = false;
//  std::cout <<"      SigProd 2   "<<std::boolalpha<<_newEventAssociated<<std::endl;
  _signals.clear();
  _goodHit = 0;
//  std::cout <<"      SigProd 3  "<<(_signalextractor.get())<<std::endl;
  _signalextractor->associate(evt);
//  std::cout <<"      SigProd 4"<<std::endl;
}

double SignalProducer::firstGood(const std::pair<double,double>& range)
{
  using namespace std;
//	cout <<"Sigproducer::firstGood(): "<<boolalpha<< _goodHitExtracted << endl;
	if(!_goodHitExtracted)
  {
    _goodHitExtracted = true;
    signals_t &sigs (output());
//    cout<< "SigProducer::firstGood(): size "<<sigs.size()<< " range '"<<range.first<<"' to '"<<range.second<<"'"<<endl;
    signals_t::iterator sigIt(find_if(sigs.begin(),sigs.end(), isInTimeRange(range)));
//    cout << "SigProducer::firstGood(): found a signal "<<boolalpha<<(sigIt != sigs.end())<<endl;
    _goodHit = (sigIt != sigs.end())? (*sigIt)["time"] : 0;
//    cout << "SigProducer::firstGood(): time: "<<_goodHit<<endl;
  }
  return _goodHit;
}

double SignalProducer::firstGood()
{
  using namespace std;
//	cout <<"Sigproducer::firstGood(): "<<boolalpha<< _goodHitExtracted << endl;
	if(!_goodHitExtracted)
  {
    _goodHitExtracted = true;
    signals_t &sigs (output());
//    cout<< "SigProducer::firstGood(): size "<<sigs.size()<< " range '"<<range.first<<"' to '"<<range.second<<"'"<<endl;
    signals_t::iterator sigIt(find_if(sigs.begin(),sigs.end(), isInTimeRange(_range)));
//    cout << "SigProducer::firstGood(): found a signal "<<boolalpha<<(sigIt != sigs.end())<<endl;
    _goodHit = (sigIt != sigs.end())? (*sigIt)["time"] : 0;
//    cout << "SigProducer::firstGood(): time: "<<_goodHit<<endl;
  }
  return _goodHit;
}

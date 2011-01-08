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
//  std::cout << "copy sigprod  "<<_signalextractor.get()<<" "<<std::boolalpha<<_newEventAssociated<<std::endl;
}

SignalProducer::~SignalProducer()
{
}

void SignalProducer::loadSettings(CASSSettings &s)
{
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
//  std::cout <<"      SigProd 1"<<std::endl;
 _newEventAssociated = true;
// std::cout <<"      SigProd 2   "<<std::boolalpha<<_newEventAssociated<<std::endl;
 _signals.clear();
// std::cout <<"      SigProd 3  "<<(_signalextractor.get())<<std::endl;
 _signalextractor->associate(evt);
// std::cout <<"      SigProd 4"<<std::endl;
}

double SignalProducer::firstGood(const std::pair<double,double>& range)
{
  using namespace std;
//  cout <<boolalpha<< _newEventAssociated << endl;
  if(_newEventAssociated)
  {
    signals_t &sigs (output());
//    cout<< "size "<<sigs.size()<<endl;
    signals_t::iterator sigIt(find_if(sigs.begin(),sigs.end(), isInTimeRange(range)));
    _goodHit = (sigIt != sigs.end())? (*sigIt)["time"] : 0;
  }
  return _goodHit;
}

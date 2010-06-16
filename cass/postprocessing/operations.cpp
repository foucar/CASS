// Copyright (C) 2010 Lutz Foucar
// (C) 2010 Thomas White - Updated to new PP framework

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"


// ********* Postprocessor 1: Compare histogram for less than constant *********

void cass::pp1::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get the value to compare against
  _value = settings.value("Value",0).toFloat();

  // Get the input
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);
  if (!_one) return;

  // Create result
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  // Gloat about how fantastic we are
  std::cout << "PostProcessor " << _key
      << ": will compare whether hist in PostProcessor " << keyOne
      << " is smaller than " << _value
      << std::endl;
}

void cass::pp1::process(const CASSEvent& evt)
{
  using namespace std;

  // Get and lock input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum values, then unlock
  one.lock.lockForRead();
  float first (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  // Compare and write result
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = first < _value;
  _result->lock.unlock();
}



// ******** Postprocessor 2: Compare histogram for greater than constant *******

void cass::pp2::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get value for comparison
  _value = settings.value("Value",0).toFloat();

  // Get input
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);
  if (!_one) return;

  // Create output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor "<<_key
      <<": will compare whether hist in PostProcessor "<<_one->key()
      <<" is greater than "<<_value
      <<std::endl;
}

void cass::pp2::process(const CASSEvent& evt)
{
  // Get and lock input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum values
  one.lock.lockForRead();
  float first (std::accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  // Compare and write result
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = first > _value;
  _result->lock.unlock();
}






// ********** Postprocessor 3: Compare histogram for equal to constant *********

void cass::pp3::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // What is the value with which we will compare?
  _value = settings.value("Value",0).toFloat();

  // Where will the input come from?
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);
  if (!_one) return;

  // Where will the result be stored?
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will compare whether hist in PostProcessor " << _one->key()
      << " is equal to " << _value
      << std::endl;
}

void cass::pp3::process(const CASSEvent& evt)
{
  // Get the input data
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum values
  one.lock.lockForRead();
  float first (std::accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  // Compare and store the result
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
      std::abs(first - _value) < std::sqrt(std::numeric_limits<float>::epsilon());
  _result->lock.unlock();
}



// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************

void cass::pp4::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get input
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);
  if (!_one) return;

  // Set up output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will apply NOT to PostProcessor " << keyOne
      << std::endl;
}

void cass::pp4::process(const CASSEvent& evt)
{
  // Get the input data
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Lock both input and output, and perform negation
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = !one.isTrue();
  _result->lock.unlock();
  one.lock.unlock();
}



// *********** Postprocessor 5: Apply boolean AND to two histograms ************

void cass::pp5::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp,_key,"HistTwo", keyTwo);
  _dependencies.push_back(keyTwo);

  if ( !(_one && _two) ) return;


  if (_one->getHist(0).dimension() != _two->getHist(0).dimension())
  {
    throw std::runtime_error("PP type 5: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will AND  PostProcessor " << keyOne
      << " with PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp5::process(const CASSEvent& evt)
{
  // Get first input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Perform the AND (under appropriate locks)
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = one.isTrue() && two.isTrue();
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}






// ******* Postprocessor 6: Calculate boolean OR between two histograms ********

void cass::pp6::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp,_key,"HistTwo", keyTwo);
  _dependencies.push_back(keyTwo);

  if ( !(_one && _two) ) return;

  if (_one->getHist(0).dimension() != _two->getHist(0).dimension())
  {
    throw std::runtime_error("PP type 6: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will OR  PostProcessor " << keyOne
      << " with PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp6::process(const CASSEvent& evt)
{
  // Get first input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Perform the AND (under appropriate locks)
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = one.isTrue() || two.isTrue();
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}



// *********** Postprocessor 7: Compare histograms (for one < two) *************

void cass::pp7::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp,_key,"HistTwo", keyTwo);
  _dependencies.push_back(keyTwo);

  if ( !(_one && _two) ) return;

  if (_one->getHist(0).dimension() != _two->getHist(0).dimension())
  {
    throw std::runtime_error("PP type 6: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will check whether Histogram in PostProcessor " << keyOne
      << " is smaller than Histogram in PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp7::process(const CASSEvent &evt)
{
  // Get first input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Add up both histograms (under locks)
  one.lock.lockForRead();
  two.lock.lockForRead();
  float first (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  float second (accumulate(two.memory().begin(),
                           two.memory().end(),
                           0.f));
  two.lock.unlock();
  one.lock.unlock();

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = (first < second);
  _result->lock.unlock();
}



// *********** Postprocessor 8: Compare histograms (for one = two) *************

void cass::pp8::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp,_key,"HistTwo", keyTwo);
  _dependencies.push_back(keyTwo);

  if ( !(_one && _two) ) return;

  if (_one->getHist(0).dimension() != _two->getHist(0).dimension())
  {
    throw std::runtime_error("PP type 6: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will check whether Histogram in PostProcessor " << keyOne
      << " is equal to Histogram in PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp8::process(const CASSEvent &evt)
{
  // Get first input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Add up both histograms (under locks)
  one.lock.lockForRead();
  two.lock.lockForRead();
  float first (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  float second (accumulate(two.memory().begin(),
                           two.memory().end(),
                           0.f));
  two.lock.unlock();
  one.lock.unlock();

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
              abs(first - second) < sqrt(std::numeric_limits<float>::epsilon());
  _result->lock.unlock();
}



// ********** Postprocessor 9: Check if histogram is in given range ************

void cass::pp9::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get the range
  _range = std::make_pair(settings.value("UpperLimit",0).toFloat(),
                          settings.value("LowerLimit",0).toFloat());

  // Get the input
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
  _dependencies.push_back(keyOne);

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will check whether hist in PostProcessor " << keyOne
      << " is between " << _range.first
      << " and " << _range.second
      << std::endl;
}

void cass::pp9::process(const CASSEvent &evt)
{
 // Get the input
 const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum up the histogram under lock
  one.lock.lockForRead();
  float value (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
                                 _range.first < value &&  value < _range.second;
  _result->lock.unlock();
}



// **************** Postprocessor 20: Subtract two histograms ******************

void cass::pp20::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _fOne = settings.value("FactorOne",1.).toFloat();
  _fTwo = settings.value("FactorTwo",1.).toFloat();

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp, _key, "HistTwo", keyTwo);
  _dependencies.push_back(keyOne);

  if ( !(_one && _two) ) return;

  const HistogramFloatBase &one
                   (dynamic_cast<const HistogramFloatBase &>(_one->getHist(0)));
  const HistogramFloatBase &two
                   (dynamic_cast<const HistogramFloatBase &>(_two->getHist(0)));

  if (one.dimension() != two.dimension())
  {
    throw std::runtime_error("PP type 20: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  _result = new HistogramFloatBase(one);
  createHistList(2*cass::NbrOfWorkers);

   std::cout << "PostProcessor " << _key
      << ": will substract Histogram in PostProcessor " << keyOne
      << " from Histogram in PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp20::process(const CASSEvent &evt)
{
  // Get first input
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Subtract using transform with a special function
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one.memory().begin(), one.memory().end(),
            two.memory().begin(),
            (dynamic_cast<const HistogramFloatBase *>(_result))->memory().begin(),
            weighted_minus(_fOne,_fTwo));
  _result->lock.unlock();
  one.lock.unlock();
  two.lock.unlock();
}



// ***************** Postprocessor 21: Divide two histograms *******************

void cass::pp21::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp, _key, "HistTwo", keyTwo);
  _dependencies.push_back(keyOne);

  if ( !(_one && _two) ) return;

  // Get (empty) histograms to check dimensionality
  const HistogramFloatBase &one
                   (dynamic_cast<const HistogramFloatBase &>(_one->getHist(0)));
  const HistogramFloatBase &two
                   (dynamic_cast<const HistogramFloatBase &>(_two->getHist(0)));

  if (one.dimension() != two.dimension())
  {
    throw std::runtime_error("PP type 20: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  // Create result histogram with the right dimensionality
  _result = new HistogramFloatBase(one);
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will divide Histogram in PostProcessor " << keyOne
      << " by Histogram in PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp21::process(const CASSEvent &evt)
{
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Divide using transform with a special function
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  std::transform(one.memory().begin(), one.memory().end(), two.memory().begin(),
          (dynamic_cast<const HistogramFloatBase *>(_result))->memory().begin(),
          std::divides<float>());
  _result->lock.unlock();
  one.lock.unlock();
  two.lock.unlock();
}



// **************** Postprocessor 22: Multiply two histograms ******************

void cass::pp22::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);

  PostProcessors::key_t keyTwo;
  _two = retrieve_and_validate(_pp, _key, "HistTwo", keyTwo);
  _dependencies.push_back(keyOne);

  if ( !(_one && _two) ) return;

  // Get (empty) histograms to check dimensionality
  const HistogramFloatBase &one
                   (dynamic_cast<const HistogramFloatBase &>(_one->getHist(0)));
  const HistogramFloatBase &two
                   (dynamic_cast<const HistogramFloatBase &>(_two->getHist(0)));

  if (one.dimension() != two.dimension())
  {
    throw std::runtime_error("PP type 20: HistOne is not the same type "
                             " as HistTwo, or they have not the same size.");
  }

  // Create result histogram with the right dimensionality
  _result = new HistogramFloatBase(one);
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will divide Histogram in PostProcessor " << keyOne
      << " by Histogram in PostProcessor " << keyTwo
      << std::endl;
}

void cass::pp22::process(const CASSEvent &evt)
{
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Get second input
  const HistogramFloatBase &two
      (reinterpret_cast<const HistogramFloatBase&>((*_two)(evt)));

  // Multiply using transform with a special function
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  std::transform(one.memory().begin(), one.memory().end(), two.memory().begin(),
          (dynamic_cast<const HistogramFloatBase *>(_result))->memory().begin(),
          std::multiplies<float>());
  _result->lock.unlock();
  one.lock.unlock();
  two.lock.unlock();
}


// ************ Postprocessor 23: Multiply histogram by constant ***************

void cass::pp23::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _factor = settings.value("Factor",1).toFloat();
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp, _key, "HistOne", keyOne);
  _dependencies.push_back(keyOne);

  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will multiply Histogram in PostProcessor "<<_idHist
      <<" with "<<_factor
      <<std::endl;
}

void cass::pp23::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one.memory().begin(),
            one.memory().end(),
            _result->memory().begin(),
            bind2nd(multiplies<float>(),_factor));
  _result->lock.unlock();
  one.lock.unlock();
}










// *** postprocessors 24 Substract constant from histogram ***

cass::pp24::pp24(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp24::~pp24()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp24::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp24::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _factor = settings.value("Factor",1).toFloat();
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will multiply Histogram in PostProcessor "<<_idHist
      <<" with "<<_factor
      <<std::endl;
}

void cass::pp24::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one.memory().begin(),
            one.memory().end(),
            _result->memory().begin(),
            bind2nd(minus<float>(),_factor));
  _result->lock.unlock();
  one.lock.unlock();
}



// *** postprocessor 25 - threshold histogram ***

cass::pp25::pp25(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp25::~pp25()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp25::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp25::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _threshold = settings.value("Threshold", 0.0).toFloat();
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist)) return;
  const HistogramFloatBase *one(dynamic_cast<HistogramFloatBase*>
                             (_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will threshold Histogram in PostProcessor "<<_idHist
      <<" above "<<_threshold
      <<std::endl;
}

class threshold : public std::binary_function<float, float, float>
{
  public:
  float operator() (float value, float thresh)const
    {
       return (value > thresh) ? value : 0.0;
    }
};

void cass::pp25::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase *one(dynamic_cast<HistogramFloatBase*>
                             (_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  one.lock.lockForRead();
  _result->lock.lockForWrite();

  transform(one.memory().begin(),
            one.memory().end(),
            _result->memory().begin(),
            bind2nd(threshold(), _threshold));

  _result->lock.unlock();
  one.lock.unlock();
}












// *** postprocessors 50 projects 2d hist to 1d histo for a selected region of the axis ***

cass::pp50::pp50(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp50::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _range = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                     settings.value("UpperBound", 1e6).toFloat());
  _axis = settings.value("Axis",HistogramBackend::xAxis).toUInt();
  _normalize = settings.value("Normalize",false).toBool();
  PostProcessors::key_t keyHist;
  _pHist = retrieve_and_validate(_pp,_key,"HistName",keyHist);
  _dependencies.push_back(keyHist);
  bool ret (setupCondition());
  if (!ret && !_pHist)
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  switch (_axis)
  {
  case (HistogramBackend::xAxis):
    _range.first  = max(_range.first, one.axis()[HistogramBackend::yAxis].lowerLimit());
    _range.second = min(_range.second,one.axis()[HistogramBackend::yAxis].upperLimit());
    _result = new Histogram1DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                   one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                   one.axis()[HistogramBackend::xAxis].upperLimit());
    break;
  case (HistogramBackend::yAxis):
    _range.first  = max(_range.first, one.axis()[HistogramBackend::xAxis].lowerLimit());
    _range.second = min(_range.second,one.axis()[HistogramBackend::xAxis].upperLimit());
    _result = new Histogram1DFloat(one.axis()[HistogramBackend::yAxis].nbrBins(),
                                   one.axis()[HistogramBackend::yAxis].lowerLimit(),
                                   one.axis()[HistogramBackend::yAxis].upperLimit());
    break;
  }
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<" will project histogram of PostProcessor "<<keyHist
      <<" from "<<_range.first
      <<" to "<<_range.second
      <<" on axis "<<_axis
      <<boolalpha<<" normalize "<<_normalize
      <<std::endl;
}

void cass::pp50::process(const CASSEvent& evt)
{
  using namespace std;
  if ((*_condition)(evt).isTrue())
  {
    const Histogram2DFloat &one
        (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
    one.lock.lockForRead();
    _result->lock.lockForWrite();
    *dynamic_cast<Histogram1DFloat*>(_result) = one.project(_range,static_cast<HistogramBackend::Axis>(_axis));
    _result->lock.unlock();
    one.lock.unlock();
  }
}













// *** postprocessors 51 calcs integral over a region in 1d histo ***

cass::pp51::pp51(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp51::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _area = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                    settings.value("UpperBound", 1e6).toFloat());
  PostProcessors::key_t HistId;
  _pHist = retrieve_and_validate(_pp,_key,"HistName",HistId);
  _dependencies.push_back(HistId);
  if (!_pHist)
    return;
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->getHist(0)));
  _area.first  = max(_area.first, one.axis()[HistogramBackend::xAxis].lowerLimit());
  _area.second = min(_area.second,one.axis()[HistogramBackend::xAxis].upperLimit());
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": will create integral of 1d histogram in PostProcessor "<<HistId
      <<" from "<<_area.first
      <<" to "<<_area.second
      <<std::endl;
}

void cass::pp51::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = one.integral(_area);
  _result->lock.unlock();
  one.lock.unlock();
}






// *** postprocessors 52 calculate the radial average of a 2d hist given a centre
//     and 1 radius (in case the value is too large, the maximum reasonable value is used) ***

cass::pp52::pp52(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp52::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  PostProcessors::key_t keyHist;
  _pHist = retrieve_and_validate(_pp,_key,"HistName",keyHist);
  _dependencies.push_back(keyHist);
  bool ret (setupCondition());
  if (!ret && !_pHist)
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = (min (min_dist_x, min_dist_y));
  _result = new Histogram1DFloat(_radius,0,_radius);
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": will calculate the radial average of histogram of PostProcessor "<<keyHist
      <<" with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" maximum radius calculated from the incoming histogram "<<_radius
      <<std::endl;
}

void cass::pp52::process(const CASSEvent& evt)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  const Histogram2DFloat &one
      (reinterpret_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.radial_project(_center,_radius);
  _result->lock.unlock();
  one.lock.unlock();
}









// *** postprocessors 53 calculate the radar plot of a 2d hist given a centre
//     and 2 radii (in case the value is too large, the maximum reasonable value is used) ***
/**
 * @todo improve and generalize radial projection to account for different distance of detector to beam line
 * @todo add treatment of possible asymmetric positions of detectors to beam line
 * @todo add possibility to have circle partially outside the physical detector dimensions
 */


cass::pp53::pp53(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp53::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  PostProcessors::key_t keyHist;
  _pHist = retrieve_and_validate(_pp,_key,"HistName",keyHist);
  _dependencies.push_back(keyHist);
  bool ret (setupCondition());
  if (!ret && !_pHist)
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  const size_t max_radius (min(min_dist_x, min_dist_y));
  const float minrad_user(settings.value("MinRadius",0.).toFloat());
  const float maxrad_user(settings.value("MaxRadius",0.).toFloat());
  const size_t minrad (one.axis()[HistogramBackend::xAxis].user2hist(minrad_user));
  const size_t maxrad (one.axis()[HistogramBackend::xAxis].user2hist(maxrad_user));
  _range = make_pair(min(max_radius, minrad),
                     min(max_radius, maxrad));
  _result = new Histogram1DFloat(_nbrBins,0,360);
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": angular distribution of hist "<<keyHist
      <<" with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" minimum radius "<<settings.value("MinRadius",0.).toFloat()
      <<" maximum radius "<<settings.value("MaxRadius",512.).toFloat()
      <<" in histogram coordinates minimum radius "<<_range.first
      <<" maximum radius "<<_range.second
      <<" Histogram has "<<_nbrBins<<" Bins"
      <<std::endl;
}

void cass::pp53::process(const CASSEvent& evt)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  const Histogram2DFloat &one
      (reinterpret_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  // retrieve the projection from the 2d hist//
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.radar_plot(_center,_range, _nbrBins);
  _result->lock.unlock();
  one.lock.unlock();
}
















// *** postprocessors 54 convert a 2d plot into a r-phi representation ***

cass::pp54::pp54(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp54::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  PostProcessors::key_t keyHist;
  _pHist = retrieve_and_validate(_pp,_key,"HistName",keyHist);
  _dependencies.push_back(keyHist);
  bool ret (setupCondition());
  if (!ret && !_pHist)
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = min(min_dist_x, min_dist_y);
  _result = new Histogram2DFloat(_nbrBins,0,360,_radius,0,_radius);
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": angular distribution with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" Histogram has "<<_nbrBins<<" Bins"
      <<" the maximum Radius in histogram coordinates "<<_radius
      <<std::endl;
}

void cass::pp54::process(const CASSEvent& evt)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  const Histogram2DFloat &one
      (reinterpret_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  // retrieve the projection from the 2d hist//
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram2DFloat*>(_result) = one.convert2RPhi(_center,_radius, _nbrBins);
  _result->lock.unlock();
  one.lock.unlock();
}










// *** postprocessor 6 histograms 0D values ***

cass::pp60::pp60(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _hist(0)
{
  loadSettings(0);
}

cass::pp60::~pp60()
{
  _pp.histograms_delete(_key);
  _hist = 0;
}

cass::PostProcessors::active_t cass::pp60::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  list.push_front(_condition);
  return list;
}

void cass::pp60::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"ConditionName",_condition));
  if (OneNotvalid || TwoNotvalid)
    return;
  set1DHist(_hist,_key);
  _pp.histograms_replace(_key,_hist);
  std::cout<<"Postprocessor "<<_key
      <<": histograms values from PostProcessor "<< _idHist
      <<" condition on PostProcessor "<<_condition
      <<std::endl;
}

void cass::pp60::operator()(const CASSEvent&)
{
  using namespace std;
  const Histogram0DFloat*cond
      (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
  if (cond->isTrue())
  {
    Histogram0DFloat* one
        (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_idHist)));
    one->lock.lockForRead();
    _hist->lock.lockForWrite();
    _hist->fill(one->getValue());
    _hist->lock.unlock();
    one->lock.unlock();
  }
}











// *** postprocessor 61 averages histograms ***

cass::pp61::pp61(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _average(0)
{
  loadSettings(0);
}

cass::pp61::~pp61()
{
  _pp.histograms_delete(_key);
  _average = 0;
}

cass::PostProcessors::active_t cass::pp61::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  list.push_front(_condition);
  return list;
}

void cass::pp61::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"ConditionName",_condition));
  if (OneNotvalid || TwoNotvalid)
    return;
  unsigned average = settings.value("average", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  const HistogramFloatBase*one
      (dynamic_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
  _pp.histograms_delete(_key);
  _average = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_average);

  std::cout<<"Postprocessor "<<_key
      <<": averages histograms from PostProcessor "<< _idHist
      <<" alpha for the averaging:"<<_alpha
      <<" condition on postprocessor:"<<_condition
      <<std::endl;
}

void cass::pp61::operator()(const CASSEvent&)
{
  using namespace std;
  const Histogram0DFloat*cond
      (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
  if (cond->isTrue())
  {
    HistogramFloatBase* one
        (reinterpret_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
    one->lock.lockForRead();
    _average->lock.lockForWrite();
    ++_average->nbrOfFills();
    float scale = (1./_average->nbrOfFills() < _alpha) ?
                   _alpha :
                   1./_average->nbrOfFills();
    transform(one->memory().begin(),one->memory().end(),
              _average->memory().begin(),
              _average->memory().begin(),
              Average(scale));
    _average->lock.unlock();
    one->lock.unlock();
  }
}













// *** postprocessor 62 sums up histograms ***

cass::pp62::pp62(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _sum(0)
{
  loadSettings(0);
}

cass::pp62::~pp62()
{
  _pp.histograms_delete(_key);
  _sum = 0;
}

cass::PostProcessors::active_t cass::pp62::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  list.push_front(_condition);
  return list;
}

void cass::pp62::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"ConditionName",_condition));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase*one
      (dynamic_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
  _pp.histograms_delete(_key);
  _sum = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_sum);
  std::cout<<"Postprocessor "<<_key
      <<": sums up histograms from PostProcessor "<< _idHist
      <<" condition on postprocessor:"<<_condition
      <<std::endl;
}

void cass::pp62::operator()(const CASSEvent&)
{
  using namespace std;
  const Histogram0DFloat*cond
      (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
  if (cond->isTrue())
  {
    HistogramFloatBase* one
        (reinterpret_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
    one->lock.lockForRead();
    _sum->lock.lockForWrite();
    ++_sum->nbrOfFills();
    transform(one->memory().begin(),one->memory().end(),
              _sum->memory().begin(),
              _sum->memory().begin(),
              plus<float>());
    _sum->lock.unlock();
    one->lock.unlock();
  }
}





// *** postprocessors 63 calculate the time average of a 0d/1d/2d hist given the number
//     of samples that are going to be used in the calculation ***

cass::pp63::pp63(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _time_avg(0), _num_seen_evt(0), _when_first_evt(0), _first_fiducials(0)
{
  loadSettings(0);
}

cass::pp63::~pp63()
{
  _pp.histograms_delete(_key);
  _time_avg = 0;
  _num_seen_evt=0;
  _when_first_evt=0;
  _first_fiducials=0;
}

cass::PostProcessors::active_t cass::pp63::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  list.push_front(_condition);
  return list;
}

void cass::pp63::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"ConditionName",_condition));
  if (OneNotvalid || TwoNotvalid)
    return;
  const size_t min_time_user (settings.value("MinTime",0).toUInt());
  const size_t max_time_user (settings.value("MaxTime",300).toUInt());
  _timerange = make_pair(min_time_user,max_time_user);
  _nbrSamples=settings.value("NumberOfSamples",5).toUInt();

  const HistogramFloatBase*one
      (dynamic_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
  _pp.histograms_delete(_key);

  _time_avg = new HistogramFloatBase(*one); //(_timerange.second,_timerange.first,_timerange.second);
  _pp.histograms_replace(_key,_time_avg);
  std::cout << "PostProcessor "<<_key
      <<" will calculate the time average of histogram of PostProcessor_"<<_idHist
      <<" from now "<<settings.value("MinTime",0).toUInt()
      <<" to "<<settings.value("MaxTime",300).toUInt()
      <<" seconds   "<<_timerange.first
      <<" ; "<<_timerange.second
      <<" each bin is equivalent to up to "<< _nbrSamples
      <<" measurements,"
      <<" condition on postprocessor:"<<_condition
      <<std::endl;
}

void cass::pp63::operator()(const cass::CASSEvent& evt)
{
  using namespace std;
  //#define debug1
#ifdef debug1
  char timeandday[40];
  struct tm * timeinfo;
#endif
  uint32_t fiducials;
  time_t now_of_event;

  const Histogram0DFloat*cond
      (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
  if (cond->isTrue())
  {
    HistogramFloatBase* one
        (reinterpret_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
    //retrieve the projection from the 0d/1d/2d hist//
    one->lock.lockForRead();
    _time_avg->lock.lockForWrite();

    // using docu at http://asg.mpimf-heidelberg.mpg.de/index.php/Howto_retrieve_the_Bunch-/Event-id
    //remember the time of the first event
    if(_when_first_evt==0)
    {
      _when_first_evt=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
      timeinfo = localtime ( &_when_first_evt );
      strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
      cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
#endif
      _num_seen_evt=1;
      _first_fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
    }
    now_of_event=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
    timeinfo = localtime ( &now_of_event );
    strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
#endif
    fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
#ifdef debug1
      timeinfo = localtime ( &now_of_event );
      strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
      //      cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
      cout<<"ora "<< timeandday<<" "<<_first_fiducials << " " <<fiducials << " " << _num_seen_evt <<endl;
#endif
    if(fiducials>_first_fiducials+(_nbrSamples-1)*4608 /*0x1200 ??why*/
        && now_of_event>=_when_first_evt)
    {
#ifdef debug1
      cout <<"time to forget"<<endl;
#endif
     //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
     _first_fiducials=fiducials;
      _when_first_evt=now_of_event;
      _num_seen_evt=1;
    }

    //    if(now_of_event<_when_first_evt-100)
    if(_first_fiducials>fiducials)
    {
#ifdef debug1
      cout <<"extra time to forget"<<endl;
#endif
     //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
     _first_fiducials=fiducials;
      _when_first_evt=now_of_event;
      _num_seen_evt=1;
    }

    ++_time_avg->nbrOfFills();
    transform(one->memory().begin(),one->memory().end(),
              _time_avg->memory().begin(),
              _time_avg->memory().begin(),
              TimeAverage(float(_num_seen_evt)));
    ++_num_seen_evt;
    if(_num_seen_evt>_nbrSamples+1) cout<<"How... it smells like fish! "<< 
                                      _num_seen_evt<< " " << _nbrSamples <<endl;
    _time_avg->lock.unlock();
    one->lock.unlock();
  }
}






// ***  pp 64 takes a 0d histogram (value) as input and writes it in the last bin of a 1d histogram
//    *** while shifting all other previously saved values one bin to the left.

cass::pp64::pp64(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp64::~pp64()
{
  _pp.histograms_delete(_key);
}

cass::PostProcessors::active_t cass::pp64::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  list.push_front(_condition);
  return list;
}

void cass::pp64::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"ConditionName",_condition));
  if (OneNotvalid || TwoNotvalid)
    return;
  _size = (settings.value("Size",10000).toUInt());

  _result = new Histogram1DFloat(_size, 0, _size-1);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<" condition on postprocessor:"<<_condition
      <<std::endl;
}

void cass::pp64::operator()(const cass::CASSEvent& evt)
{
  using namespace std;
  const Histogram0DFloat*cond
      (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
  if (cond->isTrue())
  {
    HistogramFloatBase* one
        (reinterpret_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
    one->lock.lockForRead();
    _result->lock.lockForWrite();
    ++_result->nbrOfFills();
    std::rotate(_result->memory().begin(), _result->memory().begin()+1, _result->memory().end() );
    _result->memory()[_size-1] = dynamic_cast<Histogram0DFloat*>(one)->getValue();
    _result->lock.unlock();
    one->lock.unlock();
  }
}







// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

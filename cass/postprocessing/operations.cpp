// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>


#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"




// *** postprocessor 1 compare histo for less than constant ***

cass::pp1::pp1(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp1::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _value = settings.value("Value",0).toFloat();
  PostProcessors::key_t keyOne;
  _one = retrieve_and_validate(_pp,_key,"HistOne",keyOne);
  _dependencies.push_back(keyOne);
  if (!_one)
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": will compare whether hist in PostProcessor "<<keyOne
      <<" is smaller than "<<_value
      <<std::endl;
}

void cass::pp1::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase &one
      (reinterpret_cast<const HistogramFloatBase&>((*_one)(evt)));
  one.lock.lockForRead();
  float first (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = first < _value;
  _result->lock.unlock();
}










// *** postprocessor 2 compare histo for greater than constant ***

cass::pp2::pp2(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp2::~pp2()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp2::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  return list;
}

void cass::pp2::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _value = settings.value("Value",0).toFloat();
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will compare whether hist in PostProcessor "<<_idOne
      <<" is greater than "<<_value
      <<std::endl;
}

void cass::pp2::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (reinterpret_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  float first (accumulate(one->memory().begin(),
                          one->memory().end(),
                          0.f));
  one->lock.unlock();
  _result->lock.lockForWrite();
  *_result = first > _value;
  _result->lock.unlock();
}











// *** postprocessor 3 compare histo for equal to constant ***

cass::pp3::pp3(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp3::~pp3()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp3::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  return list;
}

void cass::pp3::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _value = settings.value("Value",0).toFloat();
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will compare whether hist in PostProcessor "<<_idOne
      <<" is equal to "<<_value
      <<std::endl;
}

void cass::pp3::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (reinterpret_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  float first (accumulate(one->memory().begin(),
                          one->memory().end(),
                          0.f));
  one->lock.unlock();
  _result->lock.lockForWrite();
  *_result = abs(first - _value) < sqrt(numeric_limits<float>::epsilon());
  _result->lock.unlock();
}










// *** postprocessor 4 apply boolean NOT to 0D Histogram ***

cass::pp4::pp4(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp4::~pp4()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp4::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  return list;
}

void cass::pp4::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  const Histogram0DFloat *one
      (dynamic_cast<Histogram0DFloat*>(histogram_checkout(_idOne)));
  if(one->dimension() != 0)
    throw std::runtime_error("pp type 4: idOne is not a 0d Histogram");
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will apply NOT to PostProcessor "<<_idOne
      <<std::endl;
}

void cass::pp4::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  Histogram0DFloat *one
      (reinterpret_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  *_result = (!one->isTrue());
  _result->lock.unlock();
  one->lock.unlock();
}










// *** postprocessor 5 boolean AND of two histos ***

cass::pp5::pp5(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp5::~pp5()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp5::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp5::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if (one->dimension() != two->dimension())
  {
    throw std::runtime_error("pp type 5: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will AND  PostProcessor "<<_idOne
      <<" with PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp5::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  Histogram0DFloat *one
      (reinterpret_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  Histogram0DFloat *two
      (reinterpret_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  *_result = one->isTrue() && two->isTrue();
  _result->lock.unlock();
  two->lock.unlock();
  one->lock.unlock();
}













// *** postprocessor 6 boolean OR of two histos ***

cass::pp6::pp6(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp6::~pp6()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp6::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp6::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if (one->dimension() != two->dimension())
  {
    throw std::runtime_error("pp type 6: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will OR  PostProcessor "<<_idOne
      <<" with PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp6::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  Histogram0DFloat *one
      (reinterpret_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  Histogram0DFloat *two
      (reinterpret_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  *_result = one->isTrue() || two->isTrue();
  _result->lock.unlock();
  two->lock.unlock();
  one->lock.unlock();
}








// *** postprocessor 7 compare two histos for less ***

cass::pp7::pp7(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp7::~pp7()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp7::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp7::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp type 7: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will check whether Histogram in PostProcessor "<<_idOne
      <<" is smaller than Histogram in PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp7::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  float first (accumulate(one->memory().begin(),
                          one->memory().end(),
                          0.f));
  float second (accumulate(one->memory().begin(),
                           one->memory().end(),
                           0.f));
  two->lock.unlock();
  one->lock.unlock();
  _result->lock.lockForWrite();
  *_result = (first < second);
  _result->lock.unlock();
}













// *** postprocessors 8 compare two histos for equality ***

cass::pp8::pp8(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp8::~pp8()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp8::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp8::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp801 idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will check whether Histogram in PostProcessor "<<_idOne
      <<" is equal to Histogram in PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp8::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  float first (accumulate(one->memory().begin(),
                          one->memory().end(),
                          0.f));
  float second (accumulate(one->memory().begin(),
                           one->memory().end(),
                           0.f));
  one->lock.unlock();
  two->lock.unlock();
  _result->lock.lockForWrite();
  *_result = abs(first - second) < sqrt(numeric_limits<float>::epsilon());
  _result->lock.unlock();
}












// *** postprocessor 9 check whether histogram is in range ***

cass::pp9::pp9(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp9::~pp9()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp9::dependencies()
{
  PostProcessors::active_t  list;
  list.push_front(_idOne);
  return list;
}

void cass::pp9::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _range = make_pair(settings.value("UpperLimit",0).toFloat(),
                     settings.value("LowerLimit",0).toFloat());
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will check whether hist in PostProcessor "<<_idOne
      <<" is between "<<_range.first
      <<" and "<<_range.second
      <<std::endl;
}

void cass::pp9::operator()(const CASSEvent&)
{
  using namespace std;
  // retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (reinterpret_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  // substract using transform with a special build function//
  one->lock.lockForRead();
  float value (accumulate(one->memory().begin(),
                          one->memory().end(),
                          0.f));
  one->lock.unlock();
  _result->lock.lockForWrite();
  *_result = _range.first < value &&  value < _range.second;
  _result->lock.unlock();
}
















// *** postprocessors 20 substract two histograms ***

cass::pp20::pp20(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp20::~pp20()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp20::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp20::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _fOne = settings.value("FactorOne",1.).toFloat();
  _fTwo = settings.value("FactorTwo",1.).toFloat();
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp type 20: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will substract Histogram in PostProcessor "<<_idOne
      <<" from Histogram in PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp20::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            two->memory().begin(),
            _result->memory().begin(),
            weighted_minus(_fOne,_fTwo));
  _result->lock.unlock();
  one->lock.unlock();
  two->lock.unlock();
}

















// *** postprocessors 21 divides two histograms ***

cass::pp21::pp21(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp21::~pp21()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp21::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp21::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp type 21: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will divide Histogram in PostProcessor "<<_idOne
      <<" by Histogram in PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp21::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            two->memory().begin(),
            _result->memory().begin(),
            divides<float>());
  _result->lock.unlock();
  one->lock.unlock();
  two->lock.unlock();
}

















// *** postprocessors 22 multiplies two histograms ***

cass::pp22::pp22(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp22::~pp22()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp22::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp22::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistOne",_idOne));
  bool TwoNotvalid (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo));
  if (OneNotvalid || TwoNotvalid)
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp type 22: idOne is not the same type as idTwo or they have not the same size");
  }
  _pp.histograms_delete(_key);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will multiply Histogram in PostProcessor "<<_idOne
      <<" with Histogram in PostProcessor "<<_idTwo
      <<std::endl;
}

void cass::pp22::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            two->memory().begin(),
            _result->memory().begin(),
            multiplies<float>());
  _result->lock.unlock();
  one->lock.unlock();
  two->lock.unlock();
}



















// *** postprocessors 23 multiplies histogram with constant ***

cass::pp23::pp23(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp23::~pp23()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp23::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp23::loadSettings(size_t)
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

void cass::pp23::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            _result->memory().begin(),
            bind2nd(multiplies<float>(),_factor));
  _result->lock.unlock();
  one->lock.unlock();
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
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            _result->memory().begin(),
            bind2nd(minus<float>(),_factor));
  _result->lock.unlock();
  one->lock.unlock();
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

  one->lock.lockForRead();
  _result->lock.lockForWrite();

  transform(one->memory().begin(),
            one->memory().end(),
            _result->memory().begin(),
            bind2nd(threshold(), _threshold));

  _result->lock.unlock();
  one->lock.unlock();
}












// *** postprocessors 50 projects 2d hist to 1d histo for a selected region of the axis ***

cass::pp50::pp50(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _projec(0)
{
  loadSettings(0);
}

cass::pp50::~pp50()
{
  _pp.histograms_delete(_key);
  _projec = 0;
}

cass::PostProcessors::active_t cass::pp50::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
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
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const Histogram2DFloat *one
      (dynamic_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _pp.histograms_delete(_key);
  switch (_axis)
  {
  case (HistogramBackend::xAxis):
    _range.first  = max(_range.first, one->axis()[HistogramBackend::yAxis].lowerLimit());
    _range.second = min(_range.second,one->axis()[HistogramBackend::yAxis].upperLimit());
    _projec = new Histogram1DFloat(one->axis()[HistogramBackend::xAxis].nbrBins(),
                                   one->axis()[HistogramBackend::xAxis].lowerLimit(),
                                   one->axis()[HistogramBackend::xAxis].upperLimit());
    break;
  case (HistogramBackend::yAxis):
    _range.first  = max(_range.first, one->axis()[HistogramBackend::xAxis].lowerLimit());
    _range.second = min(_range.second,one->axis()[HistogramBackend::xAxis].upperLimit());
    _projec = new Histogram1DFloat(one->axis()[HistogramBackend::yAxis].nbrBins(),
                                   one->axis()[HistogramBackend::yAxis].lowerLimit(),
                                   one->axis()[HistogramBackend::yAxis].upperLimit());
    break;
  }
  _pp.histograms_replace(_key,_projec);
  std::cout << "PostProcessor "<<_key
      <<" will project histogram of PostProcessor "<<_idHist
      <<" from "<<_range.first
      <<" to "<<_range.second
      <<" on axis "<<_axis
      <<boolalpha<<" normalize "<<_normalize
      <<std::endl;
}

void cass::pp50::operator()(const CASSEvent&)
{
  using namespace std;
  Histogram2DFloat *one
      (dynamic_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  one->lock.lockForRead();
  _projec->lock.lockForWrite();
  *_projec = one->project(_range,static_cast<HistogramBackend::Axis>(_axis));
  _projec->lock.unlock();
  one->lock.unlock();
}













// *** postprocessors 51 calcs integral over a region in 1d histo ***

cass::pp51::pp51(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp51::~pp51()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp51::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp51::loadSettings(size_t)
{

  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _area = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                    settings.value("UpperBound", 1e6).toFloat());
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const Histogram1DFloat *one
      (dynamic_cast<Histogram1DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _area.first  = max(_area.first, one->axis()[HistogramBackend::xAxis].lowerLimit());
  _area.second = min(_area.second,one->axis()[HistogramBackend::xAxis].upperLimit());
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_key,_result);
  std::cout << "PostProcessor "<<_key
      <<": will create integral of 1d histogram in PostProcessor "<<_idHist
      <<" from "<<_area.first
      <<" to "<<_area.second
      <<std::endl;
}

void cass::pp51::operator()(const CASSEvent&)
{
  using namespace std;
  Histogram1DFloat *one
      (dynamic_cast<Histogram1DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  *_result = one->integral(_area);
  _result->lock.unlock();
  one->lock.unlock();
}






// *** postprocessors 52 calculate the radial average of a 2d hist given a centre
//     and 1 radius (in case the value is too large, the maximum reasonable value is used) ***

cass::pp52::pp52(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _projec(0)
{
  loadSettings(0);
}

cass::pp52::~pp52()
{
  _pp.histograms_delete(_key);
  _projec = 0;
}

cass::PostProcessors::active_t cass::pp52::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp52::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const HistogramFloatBase*one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _center = make_pair(one->axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one->axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one->axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one->axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = (min (min_dist_x, min_dist_y));
  _pp.histograms_delete(_key);
  _projec = new Histogram1DFloat(_radius,0,_radius);
  _pp.histograms_replace(_key,_projec);
  std::cout << "PostProcessor "<<_key
      <<": will calculate the radial average of histogram of PostProcessor "<<_idHist
      <<" with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" maximum radius calculated from the incoming histogram "<<_radius
      <<std::endl;
}

void cass::pp52::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  Histogram2DFloat *one
      (reinterpret_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //retrieve the projection from the 2d hist//
  one->lock.lockForRead();
  _projec->lock.lockForWrite();
  *_projec = one->radial_project(_center,_radius);
  _projec->lock.unlock();
  one->lock.unlock();
}









// *** postprocessors 53 calculate the radar plot of a 2d hist given a centre
//     and 2 radii (in case the value is too large, the maximum reasonable value is used) ***
/**
 * @todo improve and generalize radial projection to account for different distance of detector to beam line
 * @todo add treatment of possible asymmetric positions of detectors to beam line
 * @todo add possibility to have circle partially outside the physical detector dimensions
 */


cass::pp53::pp53(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _projec(0)
{
  loadSettings(0);
}

cass::pp53::~pp53()
{
  _pp.histograms_delete(_key);
  _projec = 0;
}

cass::PostProcessors::active_t cass::pp53::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp53::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
  _center = make_pair(one->axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one->axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one->axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one->axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  const size_t max_radius (min(min_dist_x, min_dist_y));
  const float minrad_user(settings.value("MinRadius",0.).toFloat());
  const float maxrad_user(settings.value("MaxRadius",0.).toFloat());
  const size_t minrad (one->axis()[HistogramBackend::xAxis].user2hist(minrad_user));
  const size_t maxrad (one->axis()[HistogramBackend::xAxis].user2hist(maxrad_user));
  _range = make_pair(min(max_radius, minrad),
                     min(max_radius, maxrad));
  _pp.histograms_delete(_key);
  _projec = new Histogram1DFloat(_nbrBins,0,360);
  _pp.histograms_replace(_key,_projec);
  std::cout << "PostProcessor "<<_key
      <<": angular distribution with xcenter "<<settings.value("XCenter",512).toFloat()
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

void cass::pp53::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  Histogram2DFloat *one
      (reinterpret_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  // retrieve the projection from the 2d hist//
  one->lock.lockForRead();
  _projec->lock.lockForWrite();
  *_projec = one->radar_plot(_center,_range, _nbrBins);
  _projec->lock.unlock();
  one->lock.unlock();
}
















// *** postprocessors 54 convert a 2d plot into a r-phi representation ***

cass::pp54::pp54(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _projec(0)
{
  loadSettings(0);
}

cass::pp54::~pp54()
{
  _pp.histograms_delete(_key);
  _projec = 0;
}

cass::PostProcessors::active_t cass::pp54::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp54::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  if (!retrieve_and_validate(_pp,_key,"HistName",_idHist))
    return;
  const HistogramFloatBase *one
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
  _center = make_pair(one->axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one->axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one->axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one->axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = min(min_dist_x, min_dist_y);
  _pp.histograms_delete(_key);
  _projec = new Histogram2DFloat(_nbrBins,0,360,_radius,0,_radius);
  _pp.histograms_replace(_key,_projec);
  std::cout << "PostProcessor "<<_key
      <<": angular distribution with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" Histogram has "<<_nbrBins<<" Bins"
      <<" the maximum Radius in histogram coordinates "<<_radius
      <<std::endl;
}

void cass::pp54::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  Histogram2DFloat *one
      (reinterpret_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  // retrieve the projection from the 2d hist//
  one->lock.lockForRead();
  _projec->lock.lockForWrite();
  *_projec = one->convert2RPhi(_center,_radius, _nbrBins);
  _projec->lock.unlock();
  one->lock.unlock();
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

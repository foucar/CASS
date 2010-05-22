// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>


#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"


namespace cass
{
  /** binary function for weighted substracting.
   *
   * @author Lutz Foucar
   */
  class weighted_minus : std::binary_function<float, float, float>
  {
  public:
    /** constructor.
     *
     * @param first_weight the weight value of the first substrant
     * @param second_weight the weight value of the second substrant
     */
    weighted_minus(float first_weight, float second_weight)
      :_first_weight(first_weight),_second_weight(second_weight)
    {}
    /** operator */
    float operator() (float first, float second)
    { return first * _first_weight - second * _second_weight;}
  protected:
    float _first_weight, _second_weight;
  };

  bool retrieve_and_validate(cass::PostProcessors &pp,
                             cass::PostProcessors::key_t key,
                             const char * param_name,
                             cass::PostProcessors::key_t &dependid)
  {
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(key.c_str());
    dependid = settings.value(param_name,"0").toString().toStdString();
    //when histogram id is not yet on list we return false
    try
    {
      pp.validate(dependid);
    }
    catch (InvalidHistogramError&)
    {
      return false;
    }
    return true;
  }

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
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo))
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
    throw std::runtime_error("pp800 idOne is not the same type as idTwo or they have not the same size");
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
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo))
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
  *_result = (first == second);
  _result->lock.unlock();
}










// *** postprocessors 106 substract two histograms ***

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
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo))
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
    throw std::runtime_error("pp106 idOne is not the same type as idTwo or they have not the same size");
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
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo))
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
    throw std::runtime_error("pp802 idOne is not the same type as idTwo or they have not the same size");
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
  if (!retrieve_and_validate(_pp,_key,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_key,"HistTwo",_idTwo))
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
    throw std::runtime_error("pp803 idOne is not the same type as idTwo or they have not the same size");
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
  if (!retrieve_and_validate(_pp,_key,"HistId",_idHist))
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
  if (!retrieve_and_validate(_pp,_key,"HistId",_idHist))
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
  if (!retrieve_and_validate(_pp,_key,"HistId",_idHist))
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

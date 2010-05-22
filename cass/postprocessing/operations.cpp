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

  /** function to retrieve and validate a postprocessors dependency
   * @return true when the dependcy exists
   * @param[in] id the id of the postprocessor asking for another postprocessors id
   * @param[in] param_name paramenter name of the dependency in qsettings
   * @param[out] dependid reference to the pp id that we retrieve from qsettings
   */
  bool retrieve_and_validate(PostProcessors& pp,
                             cass::PostProcessors::id_t id,
                             const char * param_name,
                             cass::PostProcessors::id_t &dependid)
  {
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(QString("p") + QString::number(id));
    dependid = ( static_cast<PostProcessors::id_t>(settings.value(param_name,0).toInt()));
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












// *** postprocessors 106 substract two histograms ***

cass::pp106::pp106(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp106::~pp106()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp106::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp106::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _fOne = settings.value("FactorOne",1.).toFloat();
  _fTwo = settings.value("FactorTwo",1.).toFloat();

  if (!retrieve_and_validate(_pp,_id,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_id,"HistTwo",_idTwo))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  //check whether they are the same type//
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp106 idOne is not the same type as idTwo or they have not the same size");
  }

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will substract Histogram in PostProcessor_"<<_idOne
      <<" from Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}

void cass::pp106::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
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













// *** postprocessors 800 compare two histos for less ***

cass::pp800::pp800(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp800::~pp800()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp800::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp800::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  if (!retrieve_and_validate(_pp,_id,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_id,"HistTwo",_idTwo))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  //check whether they are the same type//
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp800 idOne is not the same type as idTwo or they have not the same size");
  }

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will check whether Histogram in PostProcessor_"<<_idOne
      <<" is smaller than Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}

void cass::pp800::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
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














// *** postprocessors 801 compare two histos for equality ***

cass::pp801::pp801(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp801::~pp801()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp801::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp801::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  if (!retrieve_and_validate(_pp,_id,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_id,"HistTwo",_idTwo))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  //check whether they are the same type//
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp801 idOne is not the same type as idTwo or they have not the same size");
  }

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will check whether Histogram in PostProcessor_"<<_idOne
      <<" is equal to Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}

void cass::pp801::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
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
















// *** postprocessors 802 divides two histograms ***

cass::pp802::pp802(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp802::~pp802()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp802::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp802::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  if (!retrieve_and_validate(_pp,_id,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_id,"HistTwo",_idTwo))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  //check whether they are the same type//
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp802 idOne is not the same type as idTwo or they have not the same size");
  }

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will divide Histogram in PostProcessor_"<<_idOne
      <<" by Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}

void cass::pp802::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
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

















// *** postprocessors 803 multiplies two histograms ***

cass::pp803::pp803(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp803::~pp803()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp803::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idOne);
  list.push_front(_idTwo);
  return list;
}

void cass::pp803::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  if (!retrieve_and_validate(_pp,_id,"HistOne",_idOne))
    return;
  if (!retrieve_and_validate(_pp,_id,"HistTwo",_idTwo))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  const HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();
  //check whether they are the same type//
  if ((one->dimension() != two->dimension()) ||
      (one->memory().size() != two->memory().size()))
  {
    throw std::runtime_error("pp803 idOne is not the same type as idTwo or they have not the same size");
  }

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will multiply Histogram in PostProcessor_"<<_idOne
      <<" with Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}

void cass::pp803::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
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



















// *** postprocessors 804 multiplies histogram with constant ***

cass::pp804::pp804(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp804::~pp804()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp804::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idHist);
  return list;
}

void cass::pp804::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _factor = settings.value("Factor",1).toFloat();

  if (!retrieve_and_validate(_pp,_id,"HistId",_idHist))
    return;

  //retrieve histograms from the two pp//
  const HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new HistogramFloatBase(*one);
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will multiply Histogram in PostProcessor_"<<_idHist
      <<" with "<<_factor
      <<std::endl;
}

void cass::pp804::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  transform(one->memory().begin(),
            one->memory().end(),
            _result->memory().begin(),
            bind2nd(multiplies<float>(),_factor));
  _result->lock.unlock();
  one->lock.unlock();
}





















// *** postprocessors 805 calcs integral over a region in 1d histo ***

cass::pp805::pp805(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _result(0)
{
  loadSettings(0);
}

cass::pp805::~pp805()
{
  _pp.histograms_delete(_id);
  _result = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp805::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idHist);
  return list;
}

void cass::pp805::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _area = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                    settings.value("UpperBound", 1e6).toFloat());

  if (!retrieve_and_validate(_pp,_id,"HistId",_idHist))
    return;

  //make sure that lower and upper bound are not exceeding histograms boudaries
  const Histogram1DFloat *one (dynamic_cast<Histogram1DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  /** @note this will only work when the 1d histogram has already been created at this point,
   * this is not true for the waveform histograms. need to change the way waveforms are created
   */
  _area.first  = max(_area.first, one->axis()[HistogramBackend::xAxis].lowerLimit());
  _area.second = min(_area.second,one->axis()[HistogramBackend::xAxis].upperLimit());

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _result = new Histogram0DFloat();
  _pp.histograms_replace(_id,_result);

  std::cout << "PostProcessor_"<<_id
      <<" will create integral of 1d histogram in PostProcessor_"<<_idHist
      <<" from "<<_area.first
      <<" to "<<_area.second
      <<std::endl;
}

void cass::pp805::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  Histogram1DFloat *one (dynamic_cast<Histogram1DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  _result->lock.lockForWrite();
  *_result = one->integral(_area);
  _result->lock.unlock();
  one->lock.unlock();
}























// *** postprocessors 806 projects 2d hist to 1d histo for a selected region of the axis ***

cass::pp806::pp806(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _projec(0)
{
  loadSettings(0);
}

cass::pp806::~pp806()
{
  _pp.histograms_delete(_id);
  _projec = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp806::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idHist);
  return list;
}

void cass::pp806::loadSettings(size_t)
{

  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _range = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                     settings.value("UpperBound", 1e6).toFloat());
  _axis = settings.value("Axis",HistogramBackend::xAxis).toUInt();


  if (!retrieve_and_validate(_pp,_id,"HistId",_idHist))
    return;

  //make sure that lower and upper bound are not exceeding histograms boudaries
  const Histogram2DFloat *one (dynamic_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //creat the resulting histogram from the right axis of the 2d
  _pp.histograms_delete(_id);
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
  _pp.histograms_replace(_id,_projec);

  std::cout << "PostProcessor_"<<_id
      <<" will project histogram of PostProcessor_"<<_idHist
      <<" from "<<_range.first
      <<" to "<<_range.second
      <<" on axis "<<_axis
      <<std::endl;
}

void cass::pp806::operator()(const CASSEvent&)
{
  using namespace std;
  //retrieve the memory of the to be substracted histograms//
  Histogram2DFloat *one (dynamic_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();

  //retrieve the projection from the 2d hist//
  one->lock.lockForRead();
  _projec->lock.lockForWrite();
  *_projec = one->project(_range,static_cast<HistogramBackend::Axis>(_axis));
  _projec->lock.unlock();
  one->lock.unlock();
}



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
    /** operator.*/
    float operator() (float first, float second)
    { return first * _first_weight - second * _second_weight;}
  protected:
    float _first_weight, _second_weight;
  };
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

  _idOne = static_cast<PostProcessors::id_t>(settings.value("HistOne",0).toInt());
  _idTwo = static_cast<PostProcessors::id_t>(settings.value("HistTwo",0).toInt());

  //check whether our dependencies already exist
  try
  {
    _pp.validate(_idOne);
  }
  catch (InvalidHistogramError)
  {
    _reinitialize = true;
    return;
  }

  try
  {
    _pp.validate(_idTwo);
  }
  catch (InvalidHistogramError)
  {
    _reinitialize = true;
    return;
  }

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

  std::cout << "PostProcessor_"<<_id<< " will substract Histogram in PostProcessor_"<<_idOne
      <<" from Histogram in PostProcessor_"<<_idTwo
      <<std::endl;
}



void cass::pp106::operator()(const CASSEvent&)
{
  //retrieve the memory of the to be substracted histograms//
  HistogramFloatBase *one (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idOne)->second));
  _pp.histograms_release();
  HistogramFloatBase *two (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idTwo)->second));
  _pp.histograms_release();

  //substract using transform with a special build function//
  one->lock.lockForRead();
  two->lock.lockForRead();
  _result->lock.lockForWrite();
  std::transform(one->memory().begin(),
                 one->memory().end(),
                 two->memory().begin(),
                 _result->memory().begin(),
                 weighted_minus(_fOne,_fTwo));
  _result->lock.unlock();
  one->lock.unlock();
  two->lock.unlock();
}



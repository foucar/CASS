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
  : PostprocessorBackend(pp, id), _image(0)
{
  loadSettings(0);
}


cass::pp106::~pp106()
{
  _pp.histograms_delete(_id);
  _image = 0;
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

  const PostProcessors::histograms_t container (_pp.histograms_checkout());
  PostProcessors::histograms_t::const_iterator it (container.find(_idOne));
  _pp.histograms_release();

  _pp.histograms_delete(_id);
  _image = new Histogram2DFloat(it->second->axis()[HistogramBackend::xAxis].size(),
                                it->second->axis()[HistogramBackend::yAxis].size());
  _pp.histograms_replace(_id,_image);

  std::cout << "postprocessor_"<<_id<< " will substract hist "<<_idOne
      <<" from hist "<<_idTwo
      <<std::endl;
}



void cass::pp106::operator()(const CASSEvent&)
{
  const PostProcessors::histograms_t container (_pp.histograms_checkout());
  PostProcessors::histograms_t::const_iterator f(container.find(_idOne));
  HistogramFloatBase::storage_t first (dynamic_cast<Histogram2DFloat *>(f->second)->memory());
  PostProcessors::histograms_t::const_iterator s(container.find(_idTwo));
  HistogramFloatBase::storage_t second (dynamic_cast<Histogram2DFloat *>(s->second)->memory());
  _pp.histograms_release();

  f->second->lock.lockForRead();
  s->second->lock.lockForRead();
  _image->lock.lockForWrite();
  std::transform(first.begin(), first.end(),
                 second.begin(),
                 _image->memory().begin(),
                 weighted_minus(_fOne,_fTwo));
  _image->lock.unlock();
  s->second->lock.unlock();
  f->second->lock.unlock();
}



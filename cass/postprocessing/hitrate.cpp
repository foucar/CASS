// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>


#include "cass.h"
#include "hitrate.h"
#include "postprocessor.h"
#include "histogram.h"


namespace cass
{

// *** postprocessor 589 finds Single particle hits ***
cass::pp589::pp589(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0)
{
  loadSettings(0);
}

cass::pp589::~pp589()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

cass::PostProcessors::active_t cass::pp589::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp589::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  if (OneNotvalid)
    return;
  _threshold = settings.value("threshold", 1.0).toFloat();
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat;
  _pp.histograms_replace(_key,_result);

  std::cout<<"Postprocessor "<<_key
      <<": detects Single particle hits in PostProcessor "<< _idHist
      <<" threshold for detection:"<<_threshold
      <<std::endl;
}

void cass::pp589::operator()(const CASSEvent&)
{
  using namespace std;
  HistogramFloatBase* one
      (reinterpret_cast<HistogramFloatBase*>(histogram_checkout(_idHist)));
  one->lock.lockForRead();

  one->lock.unlock();
  _result->lock.lockForWrite();
  *_result = _threshold;
  _result->lock.unlock();
}




}

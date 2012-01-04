// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains postprocessors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#include <QtCore/QString>

#include "image_manipulation.h"

#include "cass_settings.h"
#include "histogram.h"

using namespace cass;
using namespace std;

// ****************** Postprocessor 40: Threshold histogram ********************

pp55::pp55(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp55::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;
  string operation(s.value("Operation","90ccw").toString().toStdString());
  if (operation == "90ccw")
  {

  }
  else if (operation == "90cw")
  {

  }
  else if (operation == "180")
  {
    _result = _one->getHist(0).clone();

  }
  else if (operation == "transpose")
  {

  }
  else if (operation == "mirrorHorizontal")
  {
    _result = _one->getHist(0).clone();

  }
  else if(operation == "mirrorVertical")
  {
    _result = _one->getHist(0).clone();

  }
  else
  {
    throw invalid_argument("pp55 (" + _key +"): Operation '" + operation +
                           "' is not supported.");
  }
  createHistList(2*NbrOfWorkers);

  cout<<endl << "PostProcessor '" << _key
      <<"' will do '"<< operation
      <<"' on Histogram in PostProcessor '" << _one->key()
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp55::histogramsChanged(const HistogramBackend* in)
{
//  QWriteLocker lock(&_histLock);
//  //return when there is no incomming histogram
//  if(!in)
//    return;
//  //return when the incomming histogram is not a direct dependant
//  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
//    return;
//  //the previous _result pointer is on the histlist and will be deleted
//  //with the call to createHistList
//  _result = in->clone();
//  createHistList(2*NbrOfWorkers);
//  //notify all pp that depend on us that our histograms have changed
//  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
//  PostProcessors::keyList_t::iterator it (dependands.begin());
//  for (; it != dependands.end(); ++it)
//    _pp.getPostProcessor(*it).histogramsChanged(_result);
//  VERBOSEOUT(cout<<"Postprocessor '"<<_key
//             <<"': histograms changed => delete existing histo"
//             <<" and create new one from input"<<endl);
}

void pp55::process(const CASSEvent &evt)
{
  // Get the input histogram
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>((*_one)(evt)));

  hist.lock.lockForRead();
  const HistogramFloatBase::storage_t& orig(hist.memory()) ;
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t::iterator pixel(
        dynamic_cast<HistogramFloatBase*>(_result)->memory().begin());
  _result->nbrOfFills()=1;
  for (size_t row(0); row < _size.second; ++row)
    for (size_t col(0); col < _size.first; ++col)
      *pixel++ = orig[_pixIdx(col,row,_size)];
  _result->lock.unlock();
  hist.lock.unlock();
}


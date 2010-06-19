// Copyright (C) 2010 Lutz Foucar

#include <algorithm>

#include "cass_event.h"
#include "histogram.h"
#include "backend.h"
#include "cass_exceptions.h"
#include "convenience_functions.h"
#include "operations.h"

using namespace cass;

PostprocessorBackend::PostprocessorBackend(PostProcessors& pp,
                                           const PostProcessors::key_t &key)
  :_key(key),
   _hide(false),
   _result(0),
   _condition(0),
   _pp(pp),
   _histLock(QReadWriteLock::Recursive)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _hide = settings.value("Hide",false).toBool();
}

PostprocessorBackend::~PostprocessorBackend()
{
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it (_histList.begin());
  for (;it != _histList.end(); ++it)
    delete it->second;
  _histList.clear();
}

const HistogramBackend& PostprocessorBackend::operator()(const CASSEvent& evt)
{
  using namespace std;
  assert(!_histList.empty());
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it
    (find_if(_histList.begin(), _histList.end(), IsKey(evt.id())));
  if(_histList.end() == it)
  {
    _result = _histList.back().second;
    histogramList_t::value_type newPair (std::make_pair(evt.id(),_result));
    _histList.pop_back();
    if (_condition && !(*_condition)(evt).isTrue())
    {
      it = _histList.begin();
      ++it;
      it =_histList.insert(it,newPair);
    }
    else
    {
      process(evt);
      _histList.push_front(newPair);
      it = _histList.begin();
    }
  }
  return *(it->second);
}

const HistogramBackend& PostprocessorBackend::getHist(const uint64_t eventid)
{
  using namespace std;
  QReadLocker lock(&_histLock);
  //if eventId is 0 then just return the latest event//
  if (0 == eventid)
    return *(_histList.front().second);
  else
  {
    histogramList_t::const_iterator it
        (find_if(_histList.begin(),_histList.end(),IsKey(eventid)));
    if (_histList.end() == it)
      throw InvalidHistogramError(eventid);
    return *(it->second);
  }
}

void PostprocessorBackend::clearHistograms()
{
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it (_histList.begin());
  for (;it != _histList.end();++it)
    it->second->clear();
}

void PostprocessorBackend::createHistList(size_t size, bool isaccumulate)
{
  using namespace std;
  if (!_result)
    throw runtime_error("HistogramBackend::createHistList: result histogram is not initalized");
  QWriteLocker lock(&_histLock);
  if (isaccumulate)
  {
    if (!_histList.empty())
      delete _histList.front().second;
  }
  else
  {
    for (histogramList_t::iterator it (_histList.begin());
    it != _histList.end();
    ++it)
      delete it->second;
  }
  _histList.clear();
  _histList.push_front (make_pair(0, _result));
  for (size_t i=1; i<size;++i)
  {
    if (isaccumulate)
      _histList.push_front (make_pair(0, _result));
    else
      _histList.push_front (make_pair(0, _result->clone()));
  }
  for (histogramList_t::iterator it (_histList.begin());
  it != _histList.end();
       ++it)
    it->second->key() = _key;
}

bool PostprocessorBackend::setupCondition()
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  if (settings.contains("ConditionName"))
  {
    _condition = setupDependency("ConditionName");
    if (!_condition)
      return false;
  }
  else
    _condition = &(_pp.getPostProcessor("DefaultTrueHist"));
  return true;
}

PostprocessorBackend* PostprocessorBackend::setupDependency(const char * depVarName)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  PostProcessors::key_t dependkey;
  dependkey = settings.value(depVarName,"").toString().toStdString();
  if (_dependencies.end() == find(_dependencies.begin(),_dependencies.end(),dependkey))
    _dependencies.push_back(dependkey);
  PostprocessorBackend *dependpp(0);
  try
  {
    dependpp = &(_pp.getPostProcessor(dependkey));
  }
  catch (InvalidPostProcessorError&)
  {
    return 0;
  }
  return dependpp;
}

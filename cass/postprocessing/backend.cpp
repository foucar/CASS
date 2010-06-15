// Copyright (C) 2010 Lutz Foucar

#include "cass_event.h"
#include "histogram.h"
#include "backend.h"
#include "cass_exceptions.h"
#include "convenience_functions.h"
#include "operations.h"

using namespace cass;

PostprocessorBackend::PostprocessorBackend(PostProcessors& pp, const PostProcessors::key_t &key, bool useCondition)
  :_key(key),
   _result(0),
   _condition(0),
   _pp(pp),
   _histLock(QReadWriteLock::Recursive)
{
  if (useCondition)
  {
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(_key.c_str());
    if (settings.contains("ConditionName"))
    {
      PostProcessors::key_t keycondition;
      _condition = retrieve_and_validate(_pp,_key,"ConditionName",keycondition);
      _dependencies.push_back(keycondition);
      if (!_condition)
        return;
    }
    else
      _condition = &(_pp.getPostProcessor("TrueHist"));
  }
  /** @note check whether this calls the overwritten function */
  loadSettings(0);
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
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it
    (find_if(_histList.begin(), _histList.end(), IsKey(evt.id())));
  if(_histList.end() == it)
  {
    _result = _histList.back().second;
    process(evt);
    histogramList_t::value_type newPair (std::make_pair(evt.id(),_result));
    _histList.push_front(newPair);
    _histList.pop_back();
    it = _histList.begin();
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

void PostprocessorBackend::createHistList(size_t size)
{
  using namespace std;
  if (!_result)
    throw runtime_error("HistogramBackend::createHistList: result histogram is not initalized");
  QWriteLocker lock(&_histLock);
  for (histogramList_t::iterator it (_histList.begin());
       it != _histList.end();
       ++it)
    delete it->second;
  _histList.clear();
  _histList.push_front (make_pair(0, _result));
  for (size_t i=1; i<size;++i)
  {
    _histList.push_front
        (make_pair(0, new HistogramFloatBase(*reinterpret_cast<HistogramFloatBase*>(_result))));
  }
}

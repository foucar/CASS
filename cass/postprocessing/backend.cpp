// Copyright (C) 2010 Lutz Foucar

/** @file backend.cpp file contains postprocessors baseclass definition
 *
 * @author Lutz Foucar
 */


#include <algorithm>
#include <tr1/functional>
#include <tr1/memory>

#include "backend.h"

#include "cass_exceptions.h"
#include "convenience_functions.h"
#include "operations.h"
#include "cass_settings.h"
#include "postprocessor.h"
#include "cass_event.h"
#include "histogram.h"

#include "log.h"

using namespace cass;
using namespace std;
using std::tr1::bind;
using std::tr1::placeholders::_1;

PostprocessorBackend::PostprocessorBackend(PostProcessors& pp,
                                           const PostProcessors::key_t &key)
  :_key(key),
   _hide(false),
   _write(true),
   _write_summary(true),
   _result(0),
   _condition(0),
   _pp(pp),
   _histLock(QReadWriteLock::Recursive)
{}

PostprocessorBackend::~PostprocessorBackend()
{
  QWriteLocker lock(&_histLock);
  if (!_histList.empty())
  {
    histogramList_t::iterator it (_histList.begin());
    HistogramBackend * old = it->second;
    delete it->second;
    ++it;
    for (;it != _histList.end(); ++it)
      if (old != it->second)
        delete it->second;
    _histList.clear();
  }
}

bool PostprocessorBackend::operator <(const PostprocessorBackend &other)
{
  return (find(_dependencies.begin(),_dependencies.end(),other.name()) == _dependencies.end());
}

const HistogramBackend& PostprocessorBackend::operator()(const CASSEvent& evt)
{
  QWriteLocker lock(&_histLock);
  assert(!_histList.empty());
  histogramList_t::iterator it(
        find_if(_histList.begin(), _histList.end(),
                bind<bool>(equal_to<uint64_t>(),evt.id(),
                           bind<uint64_t>(&histogramList_t::value_type::first,_1))));

  if(_histList.end() == it)
  {
    _result = _histList.back().second;
    /**
     * The calls that either process this event or request the condtion from
     * another postprocessor might alter the hist list (ie. if the either one
     * will resize and then call histogramsChanged). Also the _result pointer
     * might have changed, so create the pair with the pointer and the id only
     * after the call and modify the list
     */
    if (_condition && !(*_condition)(evt).isTrue())
    {
      _histList.pop_back();
      histogramList_t::value_type newPair(std::make_pair(evt.id(),_result));
      it = _histList.begin();
      ++it;
      it =_histList.insert(it,newPair);
    }
    else
    {
      process(evt);
      _histList.pop_back();
      histogramList_t::value_type newPair(std::make_pair(evt.id(),_result));
      _histList.push_front(newPair);
      it = _histList.begin();
    }
  }
  return *(it->second);
}

void PostprocessorBackend::processEvent(const CASSEvent& evt)
{
  QWriteLocker lock(&_histLock);
  assert(!_histList.empty());
  assert(find_if(_histList.begin(), _histList.end(),
                 bind(equal_to<CASSEvent::id_t>(),evt.id(),
                      bind<CASSEvent::id_t>(&histogramList_t::value_type::first,_1)))
         == _histList.end());

  histogramList_t::value_type newPair(make_pair(evt.id(),_histList.back().second));

  if (_condition->getHist(evt.id()).isTrue())
  {
    _histList.pop_back();
    _histList.push_front(newPair);
    const HistogramBackend &result(*(_histList.front().second));
    lock.unlock();
    process(evt,result);
  }
  else
  {
    _histList.pop_back();
    histogramList_t::iterator it(_histList.begin());
    ++it;
    it =_histList.insert(it,newPair);
  }
}

const HistogramBackend& PostprocessorBackend::getHist(const uint64_t eventid)
{
  typedef CASSEvent::id_t id_type;
  QWriteLocker lock(&_histLock);
  if (0 == eventid)
    return *(_histList.front().second);
  else
  {
    histogramList_t::const_iterator it
        (find_if(_histList.begin(), _histList.end(),
                 bind(equal_to<id_type>(),eventid,
                      bind<id_type>(&histogramList_t::value_type::first,_1))));
    if (_histList.end() == it)
      throw InvalidHistogramError(eventid);
    return *(it->second);
  }
}

HistogramBackend::shared_pointer PostprocessorBackend::getHistCopy(const uint64_t eventid)
{
  QWriteLocker lock(&_histLock);
  if (0 == eventid)
  {
    QReadLocker(&_histList.front().second->lock);
    return HistogramBackend::shared_pointer(_histList.front().second->copyclone());
  }
  else
  {
    histogramList_t::const_iterator it
        (find_if(_histList.begin(), _histList.end(),
                 bind<bool>(equal_to<uint64_t>(),eventid,
                            bind<uint64_t>(&histogramList_t::value_type::first,_1))));
    if (_histList.end() == it)
      throw InvalidHistogramError(eventid);
    QReadLocker(&it->second->lock);
    return HistogramBackend::shared_pointer(it->second->copyclone());
  }
}

void PostprocessorBackend::clearHistograms()
{
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it (_histList.begin());
  for (;it != _histList.end();++it)
    it->second->clear();
  histogramsChanged(0); // notify derived classes.
}

void PostprocessorBackend::createHistList(size_t size, bool isaccumulate)
{
  QWriteLocker lock(&_histLock);
  if (!_result)
  {
    throw runtime_error(string("HistogramBackend::createHistList: result ") +
                        "histogram of postprocessor '"+name()+"' is not initalized");
  }
  if (isaccumulate)
  {
    if (!_histList.empty())
      delete _histList.front().second;
  }
  else
  {
    histogramList_t::iterator it(_histList.begin());
    for (;it != _histList.end();++it)
      delete it->second;
  }
  _histList.clear();
  for (size_t i=1; i<size;++i)
  {
    if (isaccumulate)
      _histList.push_back(make_pair(0,_result));
    else
      _histList.push_back(make_pair(0,_result->clone()));
  }
  _histList.push_back(make_pair(0, _result));
  histogramList_t::iterator it(_histList.begin());
  for (;it != _histList.end();++it)
    it->second->key() = name();
}

void PostprocessorBackend::setupGeneral()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _hide = settings.value("Hide",false).toBool();
  _write = settings.value("Write",true).toBool();
  _write_summary = settings.value("WriteSummary",true).toBool();
  _comment = settings.value("Comment","").toString().toStdString();
}


bool PostprocessorBackend::setupCondition(bool conditiontype)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  if (settings.contains("ConditionName"))
  {
    _condition = setupDependency("ConditionName");
    if (!_condition)
      return false;
  }
  else
  {
    if (conditiontype)
      _condition = &(_pp.getPostProcessor("DefaultTrueHist"));
    else
      _condition = &(_pp.getPostProcessor("DefaultFalseHist"));
  }
  return true;
}

PostprocessorBackend* PostprocessorBackend::setupDependency(const char * depVarName, const PostProcessors::key_t& depkey)
{
  name_t dependkey(depkey);
  if (dependkey.empty())
  {
    CASSSettings s;
    s.beginGroup("PostProcessor");
    s.beginGroup(QString::fromStdString(name()));
    dependkey = s.value(depVarName,"").toString().toStdString();
  }
  if (dependkey == _key)
  {
    throw invalid_argument("PostprocessorBackend::setupDependency(): Error: '" + _key +
                           "' looks for a dependency '" + dependkey +
                           "'. One cannot let a postprocessor depend on itself." +
                           " Note that qsettings is not case sensitive, so on must provide" +
                           " names that differ not only in upper / lower case.");
  }

  Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): '" + _key +
           "' check if dependency key '" + depVarName + "' which is '" +
           dependkey + "'. is already on the dependency list");
  if (_dependencies.end() == find(_dependencies.begin(),_dependencies.end(),dependkey))
  {
    Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): It's not. Add it and return 0");
    _dependencies.push_back(dependkey);
    return 0;
  }

  Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): retrieve '" +
           dependkey + "' from the mananger and return a pointer to it.");
  return (&(_pp.getPostProcessor(dependkey)));
}

void PostprocessorBackend::load()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _hide = settings.value("Hide",false).toBool();
  _write = settings.value("Write",true).toBool();
  _write_summary = settings.value("WriteSummary",true).toBool();
  _comment = settings.value("Comment","").toString().toStdString();
  if (settings.contains("ConditionName"))
    _condition = setupDependency("ConditionName");
  else
    _condition = &(_pp.getPostProcessor("DefaultTrueHist"));
}

void PostprocessorBackend::process(const CASSEvent& , const HistogramBackend& )
{
  Log::add(Log::DEBUG4,"PostProcessorBackend::process(): '" + name() +
           "' process has not been implemented");
}

void PostprocessorBackend::loadSettings(size_t)
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::loadSettings(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::saveSettings(size_t)
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::saveSettings(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::aboutToQuit()
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::aboutToQuit(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::histogramsChanged(const HistogramBackend*)
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::histogramsChanged(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::processCommand(std::string )
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::processCommand(): '" + name() +
           "' not implemented");
}


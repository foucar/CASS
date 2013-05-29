// Copyright (C) 2010, 2013 Lutz Foucar

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

PostprocessorBackend::PostprocessorBackend(PostProcessors& pp, const name_t &key)
  :_key(key),
   _hide(false),
   _write(true),
   _write_summary(true),
   _result(0),
   _pp(pp),
   _histLock(QReadWriteLock::Recursive)
{}

PostprocessorBackend::~PostprocessorBackend()
{
  QWriteLocker lock(&_histLock);
//  if (!_histList.empty())
//  {
//    cachedResults_t::iterator it (_histList.begin());
//    HistogramBackend * old = it->second;
//    delete it->second;
//    ++it;
//    for (;it != _histList.end(); ++it)
//      if (old != it->second)
//        delete it->second;
  _histList.clear();
//  }
}

const HistogramBackend& PostprocessorBackend::operator()(const CASSEvent& evt)
{
//  return getHist(evt.id());
  typedef CASSEvent::id_t id_type;
  QWriteLocker lock(&_histLock);
  assert(!_histList.empty());
  cachedResults_t::iterator it(
        find_if(_histList.begin(), _histList.end(),
                bind<bool>(equal_to<id_type>(),evt.id(),
                           bind<id_type>(&cachedResult_t::first,_1))));

  if(_histList.end() == it)
  {
    _result = _histList.back().second.get();
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
      cachedResult_t newPair(make_pair(evt.id(),_result));
      it = _histList.begin();
      ++it;
      it =_histList.insert(it,newPair);
    }
    else
    {
      process(evt);
      _histList.pop_back();
      cachedResult_t newPair(std::make_pair(evt.id(),_result));
      _histList.push_front(newPair);
      it = _histList.begin();
    }
  }
  return *(it->second);
}

void PostprocessorBackend::processEvent(const CASSEvent& evt)
{
  typedef CASSEvent::id_t id_type;
  QWriteLocker listLock(&_histLock);
  assert(!_histList.empty());
  assert(find_if(_histList.begin(), _histList.end(),
                 bind(equal_to<id_type>(),evt.id(),
                      bind<id_type>(&cachedResult_t::first,_1)))
         == _histList.end());

  cachedResult_t newPair(make_pair(evt.id(),_histList.back().second));

  if (_condition->getHist(evt.id()).isTrue())
  {
    _histList.pop_back();
    _histList.push_front(newPair);
    HistogramBackend &result(*(_histList.front().second));
    /** @note this command seems to deadlock the program at some undefined point.
     *        for now, don't use it.
     */
//    QWriteLocker resultLock(&result.lock);
    listLock.unlock();
    process(evt,result);
  }
  else
  {
    _histList.pop_back();
    cachedResults_t::iterator it(_histList.begin());
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
    cachedResults_t::const_iterator it
        (find_if(_histList.begin(), _histList.end(),
                 bind(equal_to<id_type>(),eventid,
                      bind<id_type>(&cachedResult_t::first,_1))));
    if (_histList.end() == it)
      throw InvalidHistogramError(eventid);
    return *(it->second);
  }
}

HistogramBackend::shared_pointer PostprocessorBackend::getHistCopy(const uint64_t eventid)
{
  typedef CASSEvent::id_t id_type;

  QWriteLocker lock(&_histLock);
  if (0 == eventid)
  {
    QReadLocker(&_histList.front().second->lock);
    return HistogramBackend::shared_pointer(_histList.front().second->copyclone());
  }
  else
  {
    cachedResults_t::const_iterator it
        (find_if(_histList.begin(), _histList.end(),
                 bind(equal_to<id_type>(),eventid,
                      bind<id_type>(&cachedResult_t::first,_1))));
    if (_histList.end() == it)
      throw InvalidHistogramError(eventid);
    QReadLocker(&it->second->lock);
    return HistogramBackend::shared_pointer(it->second->copy_sptr());
  }
}

void PostprocessorBackend::clearHistograms()
{
  QWriteLocker lock(&_histLock);
  cachedResults_t::iterator it (_histList.begin());
  for (;it != _histList.end();++it)
    it->second->clear();
  histogramsChanged(0); // notify derived classes.
}

void PostprocessorBackend::createHistList(HistogramBackend::shared_pointer result,
                                          size_t size, bool isaccumulate)
{
  QWriteLocker lock(&_histLock);
  _histList.clear();

  result->key() = name();
  if (isaccumulate)
  {
    for (size_t i=0; i<size; ++i)
      _histList.push_back(make_pair(0,result));
  }
  else
  {
    for (size_t i=0; i<size; ++i)
    {
      HistogramBackend::shared_pointer res_cpy(result->copy_sptr());
      _histList.push_back(make_pair(0,res_cpy));
    }
  }
}

void PostprocessorBackend::createHistList(size_t size, bool isaccumulate)
{
  createHistList(_result->copy_sptr(),size,isaccumulate);
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
  }
  else
  {
    if (conditiontype)
      _condition = setupDependency("ConditionName","DefaultTrueHist");
    else
      _condition = setupDependency("ConditionName","DefaultFalseHist");
  }
  return _condition;
}

PostprocessorBackend::shared_pointer
PostprocessorBackend::setupDependency(const string &depVarName, const name_t& depkey)
{
  name_t dependkey(depkey);
  shared_pointer dependency;
  if (dependkey.empty())
  {
    CASSSettings s;
    s.beginGroup("PostProcessor");
    s.beginGroup(QString::fromStdString(name()));
    dependkey = s.value(QString::fromStdString(depVarName),"Unknown").toString().toStdString();
  }
  if (QString::fromStdString(dependkey).toUpper() == QString::fromStdString(_key).toUpper())
  {
    throw invalid_argument("PostprocessorBackend::setupDependency(): Error: '" + name() +
                           "' looks for a dependency '" + dependkey +
                           "'. One cannot let a postprocessor depend on itself." +
                           " Note that qsettings is not case sensitive, so on must provide" +
                           " names that differ not only in upper / lower case.");
  }

  Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): '" + name() +
           "' check if dependency key '" + depVarName + "' which is '" +
           dependkey + "' is already on the dependency list");
  if (_dependencies.end() == find(_dependencies.begin(),_dependencies.end(),dependkey))
  {
    Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): '" + name() +
             "': '" + dependkey +"' is not on depend list, add it ");
    _dependencies.push_back(dependkey);
  }
  else
  {
    Log::add(Log::DEBUG0,"PostprocessorBackend::setupDependency(): '" + name() +
             "' Dependency is on list. Retrieve '"+dependkey +"' from the mananger");
    dependency = _pp.getPostProcessorSPointer(dependkey);
  }
  return dependency;
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
    _condition = setupDependency("ConditionName","DefaultTrueHist");
}

void PostprocessorBackend::process(const CASSEvent& ev, HistogramBackend& result)
{
  Log::add(Log::DEBUG4,"PostProcessorBackend::process(): '" + name() +
           "' call the old process function.");
  /** @note we don't need to unlock the result, as the histogram lock can
   *        recursily be locked by the same thread
   */
  QWriteLocker lock(&_histLock);
  _result = &result;
  process(ev);
}

void PostprocessorBackend::process(const CASSEvent&)
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::process(): '" + name() +
           "' not implemented");
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


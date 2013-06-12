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

PostprocessorBackend::PostprocessorBackend(const name_t &name)
  : _name(name),
    _hide(false)
{}

PostprocessorBackend::~PostprocessorBackend()
{}

void PostprocessorBackend::processEvent(const CASSEvent& evt)
{
  CachedList::iter_type pointer(_resultList.newItem(evt.id()));
  if (_condition->result(evt.id()).isTrue())
  {
    HistogramBackend &result(*(pointer->second));
    result.lock.lockForWrite();
    process(evt,result);
    result.lock.unlock();
    _resultList.latest(pointer);
  }
}

const HistogramBackend& PostprocessorBackend::result(const CASSEvent::id_t eventid)
{
  if (0 == eventid)
    return _resultList.latest();
  else
    return _resultList.item(eventid);
}

void PostprocessorBackend::releaseEvent(const CASSEvent &event)
{
  _resultList.release(event.id());
}

HistogramBackend::shared_pointer PostprocessorBackend::resultCopy(const uint64_t eventid)
{
  return result(eventid).copy_sptr();
}

void PostprocessorBackend::clearHistograms()
{
  _resultList.clearItems();
}

void PostprocessorBackend::createHistList(HistogramBackend::shared_pointer result)
{
  result->key() = name();
  _resultList.setup(result, cass::NbrOfWorkers + 2);
}

void PostprocessorBackend::setupGeneral()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _hide = settings.value("Hide",false).toBool();
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
           "' not implemented");
}

void PostprocessorBackend::loadSettings(size_t)
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::loadSettings(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::aboutToQuit()
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::aboutToQuit(): '" + name() +
           "' not implemented");
}

void PostprocessorBackend::processCommand(std::string )
{
  Log::add(Log::DEBUG4,"PostprocessorBackend::processCommand(): '" + name() +
           "' not implemented");
}

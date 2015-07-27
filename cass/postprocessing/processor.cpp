// Copyright (C) 2010, 2013 Lutz Foucar

/** @file processor.cpp file contains postprocessors baseclass definition
 *
 * @author Lutz Foucar
 */


#include <algorithm>
#include <tr1/functional>
#include <tr1/memory>

#include "processor.h"

#include "cass_exceptions.h"
#include "convenience_functions.h"
#include "operations.h"
#include "cass_settings.h"
#include "cass_event.h"
#include "histogram.h"
#include "processor_manager.h"

#include "log.h"

using namespace cass;
using namespace std;
using std::tr1::bind;
using std::tr1::placeholders::_1;

PostProcessor::PostProcessor(const name_t &name)
  : _name(name),
    _hide(false)
{}

PostProcessor::~PostProcessor()
{}

void PostProcessor::processEvent(const CASSEvent& evt)
{
  try
  {
    CachedList::iter_type pointer(_resultList.newItem(evt.id()));
    if (_condition->result(evt.id()).isTrue())
    {
      HistogramBackend &result(*(pointer->second));
      QWriteLocker lock(&(result.lock));
      result.id() = evt.id();
      process(evt,result);
      _resultList.latest(pointer);
    }
  }
  catch (const InvalidData& error)
  {
    Log::add(Log::ERROR,"EventID '"+ toString(evt.id()) +"': "+ error.what());
  }
}

const HistogramBackend& PostProcessor::result(const CASSEvent::id_t eventid)
{
  if (0 == eventid)
    return _resultList.latest();
  else
    return _resultList.item(eventid);
}

void PostProcessor::releaseEvent(const CASSEvent &event)
{
  _resultList.release(event.id());
}

HistogramBackend::shared_pointer PostProcessor::resultCopy(const uint64_t eventid)
{
  const HistogramBackend &h(result(eventid));
  QReadLocker lock(&h.lock);
  return h.copy_sptr();
}

void PostProcessor::clearHistograms()
{
  _resultList.clearItems();
}

void PostProcessor::createHistList(HistogramBackend::shared_pointer result)
{
  result->key() = name();
  _resultList.setup(result, cass::NbrOfWorkers + 2);
}

void PostProcessor::setupGeneral()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _hide = settings.value("Hide",false).toBool();
  _comment = settings.value("Comment","").toString().toStdString();
}

bool PostProcessor::setupCondition(bool conditiontype)
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

PostProcessor::shared_pointer
PostProcessor::setupDependency(const string &depVarName, const name_t& depkey)
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
  if (QString::fromStdString(dependkey).toUpper() == QString::fromStdString(name()).toUpper())
  {
    throw invalid_argument("PostProcessor::setupDependency(): Error: '" +
                           name() + "' looks for a dependency '" + dependkey +
                           "'. One cannot let a postprocessor depend on itself." +
                           " Note that qsettings is not case sensitive, so on must provide" +
                           " names that differ not only in upper / lower case.");
  }

  Log::add(Log::DEBUG0,"PostProcessor::setupDependency(): '" + name() +
           "' check if dependency key '" + depVarName + "' which is '" +
           dependkey + "' is already on the dependency list");
  if (_dependencies.end() == find(_dependencies.begin(),_dependencies.end(),dependkey))
  {
    Log::add(Log::DEBUG0,"PostProcessor::setupDependency(): '" + name() +
             "': '" + dependkey +"' is not on depend list, add it ");
    _dependencies.push_back(dependkey);
  }
  else
  {
    Log::add(Log::DEBUG0,"PostProcessor::setupDependency(): '" + name() +
             "' Dependency is on list. Retrieve '"+dependkey +"' from the mananger");
    dependency = PostProcessors::reference().getPostProcessorSPointer(dependkey);
  }
  return dependency;
}

void PostProcessor::load()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _hide = settings.value("Hide",false).toBool();
  _comment = settings.value("Comment","").toString().toStdString();
  if (settings.contains("ConditionName"))
    _condition = setupDependency("ConditionName");
  else
    _condition = setupDependency("ConditionName","DefaultTrueHist");
}

void PostProcessor::process(const CASSEvent&, HistogramBackend&)
{
  Log::add(Log::DEBUG4,"PostProcessorBackend::process(): '" + name() +
           "' not implemented");
}

void PostProcessor::loadSettings(size_t)
{
  Log::add(Log::DEBUG4,"PostProcessor::loadSettings(): '" + name() +
           "' not implemented");
}

void PostProcessor::aboutToQuit()
{
  Log::add(Log::DEBUG4,"PostProcessor::aboutToQuit(): '" + name() +
           "' not implemented");
}

void PostProcessor::processCommand(std::string )
{
  Log::add(Log::DEBUG4,"PostProcessor::processCommand(): '" + name() +
           "' not implemented");
}

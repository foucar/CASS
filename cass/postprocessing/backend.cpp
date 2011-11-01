// Copyright (C) 2010 Lutz Foucar

/** @file backend.cpp file contains postprocessors baseclass definition
 *
 * @author Lutz Foucar
 */


#include <algorithm>

#include "cass_event.h"
#include "histogram.h"
#include "backend.h"
#include "cass_exceptions.h"
#include "convenience_functions.h"
#include "operations.h"
#include "cass_settings.h"

using namespace cass;

PostprocessorBackend::PostprocessorBackend(PostProcessors& pp,
                                           const PostProcessors::key_t &key)
  :_key(key),
   _hide(false),
   _write(true),
   _result(0),
   _condition(0),
   _pp(pp),
   _histLock(QReadWriteLock::Recursive)
{}

PostprocessorBackend::~PostprocessorBackend()
{
  QWriteLocker lock(&_histLock);
  histogramList_t::iterator it (_histList.begin());
  HistogramBackend * old = it->second;
  delete it->second;
  ++it;
  for (;it != _histList.end(); ++it)
    if (old != it->second)
      delete it->second;
  _histList.clear();
}

const HistogramBackend& PostprocessorBackend::operator()(const CASSEvent& evt)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  assert(!_histList.empty());
  histogramList_t::iterator it
    (find_if(_histList.begin(), _histList.end(), IsKey(evt.id())));
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
//    if(_condition)
//      cout <<evt.id()<<" '"<<_key << "' calling condition '"<<_condition->key()<<"'"<<endl;
    if (_condition && !(*_condition)(evt).isTrue())
    {
//      cout <<evt.id()<<" '"<<_key << "' condition '"<<_condition->key()<<"' was false "<<endl;
      _histList.pop_back();
      histogramList_t::value_type newPair(std::make_pair(evt.id(),_result));
      it = _histList.begin();
      ++it;
      it =_histList.insert(it,newPair);
    }
    else
    {
//      if(_condition)
//        cout <<evt.id()<<" '"<<_key << "' condition '"<<_condition->key()<<"' was true "<<endl;
//      else
//        cout <<evt.id()<<" '"<<_key << "' has no condition"<<endl;
      process(evt);
//      cout <<evt.id()<<" '"<< _key<< "' return from process"<<endl;
      _histList.pop_back();
      histogramList_t::value_type newPair(std::make_pair(evt.id(),_result));
      _histList.push_front(newPair);
      it = _histList.begin();
    }
  }
//  cout <<evt.id()<<" '"<< _key<< "' return from operator"<<endl;
  return *(it->second);
}

const HistogramBackend& PostprocessorBackend::getHist(const uint64_t eventid)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
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
  histogramsChanged(0); // notify derived classes.
}

void PostprocessorBackend::processCommand(std::string /*command*/)
{
  // overwrite this function in pp. can do whatever it wants to
  // do as a reaction on command.
}

void PostprocessorBackend::createHistList(size_t size, bool isaccumulate)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  if (!_result)
  {
    stringstream ss;
    ss <<"HistogramBackend::createHistList: result histogram of postprocessor '"<<_key
       <<"' is not initalized";
    throw runtime_error(ss.str());
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
    it->second->key() = _key;
}

void PostprocessorBackend::setupGeneral()
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _hide = settings.value("Hide",false).toBool();
  _write = settings.value("Write",true).toBool();
  _comment = settings.value("Comment","").toString().toStdString();
}


bool PostprocessorBackend::setupCondition(bool conditiontype)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
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

PostprocessorBackend* PostprocessorBackend::setupDependency(const char * depVarName)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  PostProcessors::key_t dependkey
      (settings.value(depVarName,"").toString().toStdString());
  if (dependkey == _key)
  {
    stringstream ss;
    ss << "PostprocessorBackend::setupDependency(): Error: '"<<_key
        <<"' looks for a dependency '"<<dependkey
        <<"'. One cannot let a postprocessor depend on itself."
        <<" Note that qsettings is not case sensitive, so on must provide"
        <<" names that differ not only in upper / lower case.";
    throw invalid_argument(ss.str());

  }
  VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): '"<<_key
             <<"' will search for key in '"<<depVarName
             <<"' which is '"<<dependkey
             <<"'. Check whether this key is already on the dependency list"
             <<endl);
  if (_dependencies.end() == find(_dependencies.begin(),_dependencies.end(),dependkey))
  {
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): dependency key '"<<dependkey
               <<"' is not on dependency list add it. This also means that we have been"
               <<" called for the first time."
               <<endl);
    _dependencies.push_back(dependkey);
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): Check whether dependency key '"<<dependkey
               <<"' does appear after our key '"<<_key
               <<"' on the active list of postprocessors."
               <<endl);
    const PostProcessors::keyList_t &activeList(_pp.activeList());
    PostProcessors::keyList_t::const_iterator myIt(find(activeList.begin(),activeList.end(),_key));
    if(find(myIt,activeList.end(),dependkey) != activeList.end())
    {
      VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): dependency key '"<<dependkey
                 <<"' does appear after '"<<_key
                 <<"' in the active list. This means its loadSettings has not been called yet"
                 <<" so we need to make sure that we exit our loadSettings before continueing"
                 <<" to load parameters by returning 0 at this point."
                 <<endl);
      return 0;
    }
#ifdef VERBOSE
    else
    {
      VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): dependency key '"<<dependkey
                 <<"' does appear before '"<<_key
                 <<"' in the active list. So its loadSettings has been called before ours."
                 <<" This means that everything is fine."
                 <<endl);
    }
#endif
  }
#ifdef VERBOSE
  else
  {
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): dependency key '"<<dependkey
               <<"' is on dependency list!"
               <<endl);
  }
#endif
  PostprocessorBackend *dependpp(0);
  try
  {
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): try to find '"<<dependkey
               <<"' in the list of postprocessors"
               <<endl);
    dependpp = &(_pp.getPostProcessor(dependkey));
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): found '"<<dependkey
               <<"' in the list of postprocessors"
               <<endl);
  }
  catch (InvalidPostProcessorError&)
  {
    VERBOSEOUT(cout <<"PostprocessorBackend::setupDependency(): did not find '"<<dependkey
               <<"' in the list of postprocessors"
               <<endl);
    return 0;
  }
  return dependpp;
}



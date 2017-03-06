// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/** @file processor.h file contains processors baseclass declaration
 *
 * @author Lutz Foucar
 */

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <QtCore/QReadWriteLock>

#include <list>
#include <string>
#include <stdint.h>
#include <utility>
#include <tr1/memory>

#include "cass.h"
#include "result.hpp"
#include "cass_event.h"
#include "cached_list.hpp"

namespace cass
{
/** base class for processors.
 *
 * This class handles most of the functionality of a processor. When
 * creating a new processor the user has just the overwrite the process
 * function. There it will retrieve the result from either other
 * processors or from the cassevent itselve. All the rest is handled by
 * the base class. Optionally, if one wants to have user interaction with the
 * class, this can be implemented by overwriting loadSettings.
 *
 * @author Lutz Foucar
 * @author Jochen Kuepper
 */
class Processor
{
public:
  /** a shared pointer of this */
  typedef std::tr1::shared_ptr<Processor> shared_pointer;

  /** define the name type */
  typedef std::string name_t;

  /** define the list of names */
  typedef std::list<name_t> names_t;

  /** define the results */
  typedef CachedList::item_type result_t;

  /** define the shared pointer to the result */
  typedef CachedList::item_sp resultsp_t;

  /** constructor
   *
   * @param name the name of the processor
   */
  Processor(const name_t &name);

  /** virtual destructor */
  virtual ~Processor();

  /** process the event
   *
   * @note this is the function that should only be called by the Processor
   *       Manager.
   * @note only use this function if all dependencies have been processed before.
   *
   * It will retrieve the pointer to the last result in the list and call
   * the process function to process the event, if the condition is true.
   * The histlist is locked throughout the the operations on the list, but it
   * will be unlocked after the result has been write lockend and before process
   * is called.
   *
   * If the condition is not true, the pointer to the result will be put to the
   * second to front position in the list.
   *
   * @param event the event to be processed
   */
  virtual void processEvent(const CASSEvent& event);

  /** retrieve a result for a given id.
   *
   * return a reference to the result for the given id or the most recent one
   * in case of eventid beeing 0
   *
   * @return const reference to the requested histogram
   * @param eventid the event id of the histogram that is requested.
   *                Default is 0
   */
  virtual const result_t& result(const CASSEvent::id_t eventid=0);

  /** tell the list that the result for event can be overwritten
   *
   * details
   *
   * @param event The event that can be released
   */
  virtual void releaseEvent(const CASSEvent &event);

  /** retrieve histogram for id
   *
   * same as getHist, but returns a copy of the histogram.
   *
   * Locks the histogram for read access before creating the copy.
   *
   * @return shared pointer of a copy of the histogram
   * @param eventid the event id of the histogram that is requested.
   *                Default is 0
   */
  result_t::shared_pointer resultCopy(const uint64_t eventid);

  /** Provide default implementation of loadSettings that does nothing
   *
   * @note the implementation of load settings should ensure that all
   *       dependencies should be known at the first time it is run
   *
   * @param unused not used
   */
  virtual void loadSettings(size_t unused);

  /** load the general settings
   *
   * loads the settings common to all procesors then calls loadSettings to
   * get the specific settings of the processor
   */
  virtual void load();

  /** function that will be called when the processor is about to be deleted */
  virtual void aboutToQuit();

  /** Define all processors keys a processor depends on
   *
   * If the dependencies are user choosable they must all be set in
   * loadSettings before it makes sense to call this function.
   *
   * This function will be called by Processors::setup() when it creates
   * the container with all activated processors.
   */
  const names_t& dependencies()
  {
    return _dependencies;
  }

  /** clear the dependenies */
  void clearDependencies() {_dependencies.clear();}

  /** clear the histograms
   *
   * this will lock for write access to the histograms before clearing them
   */
  void clearHistograms();

  /** process command in pp
   *
   * overwrite this function in pp. can do whatever it wants to do as a
   * reaction on command.
   *
   * @param command the command string transmitted
   */
  virtual void processCommand(std::string command);

  /** retrieve the name of this processor */
  const name_t name() const {return _name;}

  /** retrieve the hide flag of this processor */
  bool hide()const {return _hide;}

  /** retrieve the comment of this processor */
  const std::string& comment()const {return _comment;}

protected:
  /** process the event
   *
   * This will evaluate the event and fill the resulting histogram. It needs
   * to be implemented in the processors. The result should be locked when
   * calling this function so users can rely on the fact that they can savely
   * use the result without locking it.
   *
   * The default implementation mimiks the behaviour of the operator(). It
   * assings the result to the _result member and locks the list to ensure that
   * noone can process this at the same time and therefore change the pointer to
   * the _result member.
   *
   * @param event the cassevent to work on
   * @param result this is where the result will be written to
   */
  virtual void process(const CASSEvent& event, result_t& result);

  /** create result list.
   *
   * uses cass::CachedList::setup to generate the result list. The size is
   * 2+cass::nbrworkers.
   *
   * @param result shared pointer of the result that will be used in the cached
   *               result list
   */
  virtual void createHistList(result_t::shared_pointer result);

  /** general setup of the processor
   *
   * will setup the options that are available for all processors
   *
   * @cassttng Processor/\%name\%/{Hide} \n
   *           Flag that will hide this processor in cassview's combobox.
   *           Default is false
   * @cassttng Processor/\%name\%/{Write} \n
   *           Flag that will tell a dumper to write this processor into
   *           the file. Default is true
   * @cassttng Processor/\%name\%/{WriteSummary} \n
   *           Flag that will tell a dumper to write this processor into
   *           the summary. Useful for histograms that are only interesting
   *           per run. Default is true
   * @cassttng Processor/\%name\%/{Comment} \n
   *           A comment with a short description of what this processor
   *           is doing. Will be added to the file, when its written.
   *           Default is "".
   */
  void setupGeneral();

  /** setup the condition.
   *
   * this will setup the condition with the default name ConditionList
   *
   * @cassttng Processor/\%name\%/{ConditionName} \n
   *           0D Processor name that we check before filling image.
   *           if this setting is not defined, this processor is
   *           unconditional. Therefore its always true.
   *
   * @return true when condition is there, false otherwise
   * @param defaultConditionType the type of condition that should be used when
   *                             there is no ConditionName defined in cass.ini
   */
  bool setupCondition(bool defaultConditionType=true);

  /** setup the dependecy.
   *
   * this will look up the dependecy key in cass.ini and tries to get it from
   * the processors. It will return the pointer to the dependecy
   * processor when it is there. If it's not in the container it will
   * return 0. When the depencendy key is not already in the list with all
   * dependcies, it will be added.
   *
   * In case the second parameter is set, then it doesn't look up the key name
   * in the cass.ini, but rather use the provided one.
   *
   * @return pointer to the dependency processor
   * @param[in] depVarName the name of the setting that hold the dependcy key
   * @param[in] name optional name of the key, without getting it from the
   *                 settings file.
   */
  shared_pointer setupDependency(const std::string& depVarName, const name_t& name="");

protected:
  /** the processors name */
  const name_t _name;

  /** flag to tell whether this pp should be hidden in the dropdown list */
  bool _hide;

  /** optional comment that one can add to a processor.
   *
   * Will be used when writing this pp to file.
   */
  std::string _comment;

  /** the list of results */
  CachedList _resultList;

  /** the list of dependencies */
  names_t _dependencies;

  /** pointer to the processor that will contain the condition */
  shared_pointer _condition;
};





/** an accumulating processor
 *
 * instead of having a list of result, just uses one result.
 * Overwrites functions to only use one result
 */
class AccumulatingProcessor : public Processor
{
public:
  /** constructor
   *
   * @param name the name of the processor
   */
  AccumulatingProcessor(const name_t &name)
    : Processor(name),
      _nbrEventsAccumulated(0)
  {}

  /** virtual destructor */
  virtual ~AccumulatingProcessor() {}

  /** process the event
   *
   * @note this is the function that should only be called by the Processor
   *       Manager.
   * @note only use this function if all dependencies have been processed before.
   *
   * retrieve the result from the list. Get the writelock on it and process it,
   * if condition is true.
   *
   * @param evt the event to be processed
   */
  virtual void processEvent(const CASSEvent& evt)
  {
    if (_condition->result(evt.id()).isTrue())
    {
      QWriteLocker locker(&(_result->lock));
      _result->id(evt.id());
      process(evt,*_result);
    }
  }

  /** retrieve a result.
   *
   * return a reference to the latest result no matter what Id has been given,
   * as it doesn't make any sense to differentiate between different events.
   *
   * @return const reference to the requested histogram
   * @param eventid  Ignored
   */
  virtual const result_t& result(const CASSEvent::id_t)
  {
    return *_result;
  }

  /** overwrite default behaviour to do nothing */
  virtual void releaseEvent(const CASSEvent&){}

  /** create the list of results
   *
   * Just sets up the one result that an accumulating processor manages
   *
   * @param result shared pointer of the result that will be used in the cached
   *               result list
   */
  virtual void createHistList(result_t::shared_pointer result)
  {
    result->name(name());
    _result = result->clone();
  }

protected:
  /** the result that accumulates the events */
  result_t::shared_pointer _result;

  /** the number of events the processor has accumulated */
  size_t _nbrEventsAccumulated;
};

} //end namespace cass

#endif

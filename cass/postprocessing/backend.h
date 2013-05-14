// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/** @file backend.h file contains postprocessors baseclass declaration
 *
 * @author Lutz Foucar
 */

#ifndef _POSTPROCESSOR_BACKEND_H_
#define _POSTPROCESSOR_BACKEND_H_

#include <QtCore/QReadWriteLock>

#include <list>
#include <string>
#include <stdint.h>
#include <utility>
#include <tr1/memory>

#include "cass.h"
#include "histogram.h"
#include "cass_event.h"

namespace cass
{
//forward declaration
class PostProcessors;

/** base class for postprocessors.
 *
 * This class handles most of the functionality of a postprocessor. When
 * creating a new postprocessor the user has just the overwrite the process
 * function. There it will retrieve the result from either other
 * postprocessors or from the cassevent itselve. All the rest is handled by
 * the base class. Optionally, if one wants to have user interaction with the
 * class, this can be implemented by overwriting loadSettings.
 *
 * @author Lutz Foucar
 * @author Jochen Kuepper
 */
class PostprocessorBackend
{
public:
  /** a shared pointer of this */
  typedef std::tr1::shared_ptr<PostprocessorBackend> shared_pointer;

  /** define the name type */
  typedef std::string name_t;

  /** define the list of names */
  typedef std::list<name_t> names_t;

  /** define the list of cached results */
  /** @todo make the histogram list a list of shared pointers to the hbacks
   *        which means that one has to change all pps, since they new the
   *        _result pointer
   */
  typedef std::list<std::pair<CASSEvent::id_t, HistogramBackend*> > histogramList_t;

  /** constructor
   *
   * @param pp reference to the class that contains all postprocessors
   * @param name the name of the postprocessor
   */
  PostprocessorBackend(PostProcessors& pp, const name_t &name);

  /** virtual destructor */
  virtual ~PostprocessorBackend();

  /** compare this PostProcessor to another
   *
   * this postprocessor is smaller than the other when the other PostProcessor
   * is not on the dependecy list of this PostProcesor
   *
   * @return true when other name is not on dependency list
   * @param other the PostProcessor to compare this one to
   */
  bool operator < (const PostprocessorBackend& other);

  /** main operator
   *
   * Will be called for each event by postprocessors container. Sometimes by
   * postprocessors that depend on this pp. This function will check
   * whether this event has already been processed and if the result is on the
   * list. When its not on the list, then it will call the pure virtual
   * function process event. The latter is done only if either there is no
   * condition or the condition is true. The histogram where the result of the
   * evaluation should go to is _result. When the condition is false, then
   * it will be put into the list as second to first. So that only the first
   * element of the list contains the last processed histogram.
   *
   * Before this function does anything it will writelock itselve. After
   * finishing this function the write lock will automaticly be released.
   *
   * @return const reference to the resulting histogram
   * @param evt the cassevent to work on
   */
  virtual const HistogramBackend& operator()(const CASSEvent& evt);

  /** retrieve a histogram for a given id.
   *
   * Lock this function with a readlock. When this function returns it will
   * automaticaly release the lock.
   *
   * @note we need to use a write lock here. Otherwise on will never get the
   *       histogram, as the histogram list is almost everytime locked for
   *       write access.
   *
   * When parameter is 0 then it just returns the last known histogram. When
   * there is no histogram for the requested event id, then this function will
   * throw an invalid_argument exception.
   *
   * @return const reference to the requested histogram
   * @param eventid the event id of the histogram that is requested.
   *                Default is 0
   */
  const HistogramBackend& getHist(const CASSEvent::id_t eventid);

  /** retrieve histogram for id
   *
   * same as getHist, but returns a copy of the histogram.
   *
   * Locks the histogram for read access before creating the copy.
   *
   * @return copy of the requested histogram
   * @param eventid the event id of the histogram that is requested.
   *                Default is 0
   */
  HistogramBackend::shared_pointer getHistCopy(const uint64_t eventid);

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
   * loads the settings common to all postprocesors then calls loadSettings to
   * get the specific settings of the postprocessor
   */
  virtual void load();

  /** Provide default implementation of saveSettings that does nothing
   *
   * @param unused not used
   */
  virtual void saveSettings(size_t unused);

  /** function that will be called when the postprocessor is about to be deleted */
  virtual void aboutToQuit();

  /** Define all postprocessors keys a postprocessor depends on
   *
   * If the dependencies are user choosable they must all be set in
   * loadSettings before it makes sense to call this function.
   *
   * This function will be called by PostProcessors::setup() when it creates
   * the container with all activated postprocessors.
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

  /** histograms changed notification
   *
   * postprocessors can override this virtual function to execute
   * code after the histograms for this postprocessor have been changed.
   *
   * @param in pointer to the histogram that has changed
   */
  virtual void histogramsChanged(const HistogramBackend* in);

  /** process command in pp
   *
   * overwrite this function in pp. can do whatever it wants to do as a
   * reaction on command.
   *
   * @param command the command string transmitted
   */
  virtual void processCommand(std::string command);

  /** retrieve the key of this postprocessor */
  const name_t key() const {return _key;}

  /** retrieve the name of this postprocessor */
  const name_t name() const {return _key;}

  /** retrieve the hide flag of this postprocessor */
  bool hide()const {return _hide;}

  /** retrieve the write flag of this postprocessor */
  bool write()const {return _write;}

  /** retrieve the write summary flag of this postprocessor */
  bool write_summary()const {return _write_summary;}

  /** retrieve the comment of this postprocessor */
  const std::string& comment()const {return _comment;}

protected:
  /** process the event
   *
   * @note this is the function that should only be called by the PostProcessor
   *       Manager.
   * @note the function relies on the condition dependency havin been processed
   *       before.
   *
   * It will retrieve the pointer to the last result in the list and call
   * the process function to process the event, if the condition is true.
   * The histlist is locked throughout the the operations on the list, but it
   * will be unlocked before process is called.
   *
   * If the condition is not true, the pointer to the result will be put to the
   * second to front position in the list.
   *
   * @param ev the event to be processed
   */
  void processEvent(const CASSEvent& ev);

  /** process the event
   *
   * This will evaluate the event and fill the resulting histogram. It needs
   * to be implemented in the postprocessors.
   *
   * @param event the cassevent to work on
   * @param result this is where the result will be written to
   */
  virtual void process(const CASSEvent& ev, const HistogramBackend& result);

  /** process the event
   *
   * This will evaluate the event and fill the resulting histogram. It needs
   * to be implemented in the postprocessors.
   *HistogramBackend::shared_pointer(
   * @param event the cassevent to work on
   */
  virtual void process(const CASSEvent& event) = 0;

  /** create histogram list.
   *
   * Before creating the list the contents of the old list will be deleted.
   *
   * This function relies on that the the histogrambackened pointer (_result)
   * points to a valid histogram. It will take it to create all the other
   * histograms that will be put on the histogram list.
   *
   * When this postprocessor is an accumulating postprocessor, then it will
   * not clone the histogram _result, but create the list with pointers to
   * the same result. Default is false (non accumulating pp)
   *
   * @param[in] size The size of the list
   * @param[in] isaccumulate flag to tell this that it is a accumulating pp
   */
  void createHistList(size_t size, bool isaccumulate=false);

  /** general setup of the postprocessor
   *
   * will setup the options that are available for all postprocessors
   *
   * @cassttng PostProcessor/\%name\%/{Hide} \n
   *           Flag that will hide this postprocessor in cassview's combobox.
   *           Default is false
   * @cassttng PostProcessor/\%name\%/{Write} \n
   *           Flag that will tell a dumper to write this postprocessor into
   *           the file. Default is true
   * @cassttng PostProcessor/\%name\%/{WriteSummary} \n
   *           Flag that will tell a dumper to write this postprocessor into
   *           the summary. Useful for histograms that are only interesting
   *           per run. Default is true
   * @cassttng PostProcessor/\%name\%/{Comment} \n
   *           A comment with a short description of what this postprocessor
   *           is doing. Will be added to the file, when its written.
   *           Default is "".
   */
  void setupGeneral();

  /** setup the condition.
   *
   * this will setup the condition with the default name ConditionList
   *
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before filling image.
   *           if this setting is not defined, this postprocessor is
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
   * the postprocessors. It will return the pointer to the dependecy
   * postprocessor when it is there. If it's not in the container it will
   * return 0. When the depencendy key is not already in the list with all
   * dependcies, it will be added.
   *
   * In case the second parameter is set, then it doesn't look up the key name
   * in the cass.ini, but rather use the provided one.
   *
   * @return pointer to the dependency postprocessor
   * @param[in] depVarName the name of the setting that hold the dependcy key
   * @param[in] name optional name of the key, without getting it from the
   *                 settings file.
   */
  PostprocessorBackend* setupDependency(const char * depVarName, const name_t& name="");

protected:
  /** the postprocessors name */
  name_t _key;

  /** flag to tell whether this pp should be hidden in the dropdown list */
  bool _hide;

  /** flag to tell whether to write this pp into file */
  bool _write;

  /** flag to tell whether to write this pp into summary */
  bool _write_summary;

  /** optional comment that one can add to a postprocessor.
   * Will be used when writing this pp to file.
   */
  std::string _comment;

  /** the list of histograms - event ids */
  histogramList_t _histList;

  /** the list of dependencies */
  names_t _dependencies;

  /** pointer to the most recent histogram */
  HistogramBackend* _result;

  /** pointer to the postprocessor that will contain the condition */
  PostprocessorBackend* _condition;

  /** reference to the PostProcessors container */
  PostProcessors &_pp;

  /** histogram list lock */
  QReadWriteLock _histLock;
};

} //end namespace cass

#endif


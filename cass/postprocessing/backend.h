// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _POSTPROCESSOR_BACKEND_H_
#define _POSTPROCESSOR_BACKEND_H_

#include <QtCore/QReadWriteLock>

#include <list>
#include <string>
#include <stdint.h>
#include <utility>

#include "cass.h"
#include "postprocessor.h"

namespace cass
{
  //forward declaration
  class CASSEvent;

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
  class CASSSHARED_EXPORT PostprocessorBackend
  {
  public:
    /** constructor
     *
     * @param pp reference to the class that contains all postprocessors
     * @param key the key in the container of this postprocessor
     */
    PostprocessorBackend(PostProcessors& pp, const PostProcessors::key_t &key);

    /** virtual destructor */
    virtual ~PostprocessorBackend();

    /** typedef describing how the list of histograms works */
    typedef std::list<std::pair<uint64_t, HistogramBackend*> > histogramList_t;

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
    const HistogramBackend& operator()(const CASSEvent& evt);

    /** retrieve a histogram for a given id.
     *
     * Lock this function with a readlock. When this function returns it will
     * automaticaly release the lock.
     * When parameter is 0 then it just returns the last known histogram. When
     * there is no histogram for the requested event id, then this function will
     * throw an invalid_argument exception.
     *
     * @param eventid the event id of the histogram that is requested.
     *                Default is 0
     */
    const HistogramBackend& getHist(const uint64_t eventid);

    /** Provide default implementation of loadSettings that does nothing */
    virtual void loadSettings(size_t)
    {
      VERBOSEOUT(std::cout << "calling backend's load settings"<<std::endl);
    }

    /** Provide default implementation of saveSettings that does nothing */
    virtual void saveSettings(size_t)
    {
      VERBOSEOUT(std::cout << "calling backend's save settings"<<std::endl);
    }

    /** function that will be called when the postprocessor is about to be deleted */
    virtual void aboutToQuit() {}

    /** Define all postprocessors keys a postprocessor depends on
     *
     * If the dependencies are user choosable they must all be set in
     * loadSettings before it makes sense to call this function.
     *
     * This function will be called by PostProcessors::setup() when it creates
     * the container with all activated postprocessors.
     */
    const PostProcessors::keyList_t& dependencies()
    {
      return _dependencies;
    }

    /** clear the dependenies */
    void clearDependencies() {_dependencies.clear();}

    /** clear the histograms.
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
    virtual void histogramsChanged(const HistogramBackend* in=0)
    {
      VERBOSEOUT(std::cout<<"PostProcessor backend histogramsChanged"
                 << std::endl);
    }

    /** process command in pp
     *
     * this will lock for write access to the histograms before clearing them
     */
    virtual void processCommand(std::string command);

    /** retrieve the key of this postprocessor */
    const PostProcessors::key_t key() const {return _key;}

    /** retrieve the hide flag of this postprocessor */
    bool hide()const {return _hide;}

    /** retrieve the write flag of this postprocessor */
    bool write()const {return _write;}

    /** retrieve the comment of this postprocessor */
    const std::string& comment()const {return _comment;}

  protected:
    /** process the event
     *
     * This will evaluate the event and fill the resulting histogram. It needs
     * to be implemented in the postprocessors.
     *
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
     * @return pointer to the dependency postprocessor
     * @param[in] depVarName the name of the setting that hold the dependcy key
     */
    PostprocessorBackend* setupDependency(const char * depVarName);

  protected:
    /** the postprocessors key */
    PostProcessors::key_t _key;

    /** flag to tell whether this pp should be hidden in the dropdown list */
    bool _hide;

    /** flag to tell whether to write this pp into file */
    bool _write;

    /** optional comment that one can add to a postprocessor.
     * Will be used when writing this pp to file.
     */
    std::string _comment;

    /** the list of histograms - event ids */
    histogramList_t _histList;

    /** the list of dependencies */
    PostProcessors::keyList_t _dependencies;

    /** pointer to the most recent histogram */
    HistogramBackend *_result;

    /** pointer to the postprocessor that will contain the condition */
    PostprocessorBackend* _condition;

    /** reference to the PostProcessors container */
    PostProcessors &_pp;

    /** histogram list lock */
    QReadWriteLock _histLock;
  };

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

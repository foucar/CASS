// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _POSTPROCESSOR_BACKEND_H_
#define _POSTPROCESSOR_BACKEND_H_

#include <list>
#include <string>
#include <stdint.h>
#include <utility>

#include "cass.h"
#include "postprocessor.h"
#include "histogram.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration
  class CASSEvent;

  /** predicate class for find_if.
   * this helps finding the right key in the list of pairs eventid - Histogram
   * @author Lutz Foucar
   */
  class IsKey
  {
  public:
    /** initialize the key in the constructor*/
    IsKey(const uint64_t key):_key(key){}
    /** compares the first element of the pair to the key*/
    bool operator()(const std::pair<uint64_t,HistogramBackend*>& p)const
    { return (_key == p.first); }
  private:
    /** the key that we will compare to in the operator*/
    const uint64_t _key;
  };





  /** base class for postprocessors */
  class CASSSHARED_EXPORT PostprocessorBackend
  {
  public:
    /** constructor
     *
     * @param pp reference to the class that contains all postprocessors
     * @param key the key in the container of this postprocessor
     */
    PostprocessorBackend(PostProcessors& pp, const PostProcessors::key_t &key)
      :_key(key),
       _result(0),
       _pp(pp),
       _histLock(QReadWriteLock::Recursive)
    {
      /** @note check whether this calls the overwritten function */
      loadSettings(0);
    }

    /** virtual destructor */
    virtual ~PostprocessorBackend() {}

    /** typedef describing how the list of histograms works */
    typedef std::list<std::pair<uint64_t, HistogramBackend*> > histogramList_t;

    /** main operator
     *
     * will be called for each event by postprocesors. This function will check
     * wether this event has already been processed and if the result is on the
     * list. When its not on the list, then it will call the pure virtual
     * function process event. The histogram where the result of the
     * evaluation should go to is _result.
     * Before this function does anything it will writelock itselve. After
     * finishing this function the write lock will automaticly be released.
     * @return const reference to the resulting histogram
     * @param evt the cassevent to work on
     */
    const HistogramBackend& operator()(const CASSEvent& evt)
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

    /** retrieve a histogram for a given id.
     *
     * Lock this function with a readlock. When this function returns it will
     * automaticaly release the lock.
     * When parameter is 0 then it just returns the last known histogram. When
     * there is no histogram for the requested event id, then this function will
     * throw an invalid_argument exception.
     * @param eventid the event id of the histogram that is requested. Default is 0
     */
    const HistogramBackend& getHist()(const uint64_t eventid)
    {
      using namespace std;
      QReadLocker lock(&_histLock);
      //if eventId is 0 then just return the latest event//
      if (0 == eventId)
        return *_histList.front().second;
      else
      {
        histogramList_t::const_iterator it
            (find_if(_histList.begin(),_histList.end(),IsKey(eventid)));
        if (_histList.end() == it)
          throw InvalidHistogramError(eventId);
        return *(it.second);
      }
    }

    /** Provide default implementation of loadSettings that does nothing */
    virtual void loadSettings(size_t)
    {
      VERBOSEOUT(std::cout << "calling backend's load settings"<<std::endl);
    }

    /** Define all postprocessors we depend on
     *
     * The dependencies must be run before the actual postprocessor is run by itself.
     */
    const PostProcessors::keyList_t& dependencies()
    {
      return _dependencies;
    }

    /** clear the histograms
     * this will lock for write access to the histograms before clearing them
     */
    void clearHistograms()
    {
      QWriteLocker lock(&_histLock);
      histogramList_t::iterator it (_histList.begin());
      for (;it != _histList.end();++it)
        it->second->clear();
    }

  protected:
    /** process the event
     *
     * This will evaluate the event and fill the resulting histogram. It needs
     * to be implemented in the postprocessors.
     * @param event the cassevent to work on
     */
    virtual void process(const CASSEvent& event) = 0;

    /** create histogram list.
     *
     * This function relies on that the the histogrambackened pointer has been
     * newed. It will take it to create all the other histograms that are on
     * the histogram list.
     * @param[in] size The size of the list
     */
    void createHistList(size_t size)
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
      for (size_t i=0; i<size;++i)
      {
        _histList.push_front
            (make_pair(0, new HistogramFloatBase(*reinterpret_cast<HistogramFloatBase*>(_result))));
      }
    }

  protected:
    /** the postprocessors key */
    PostProcessors::key_t _key;

    /** the list of histograms - event ids */
    histogramList_t _histList;

    /** the list of dependencies */
    PostProcessors::keyList_t _dependencies;

    /** pointer to the most recent histogram */
    HistogramBackend *_result;

    /** reference to the PostProcessors container */
    PostProcessors &_pp;

    /** histogram list lock */
    QReadWriteLock _histlock;
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

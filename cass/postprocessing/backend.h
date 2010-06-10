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
      :_key(key), _pp(pp)
    {}

    /** virtual destructor */
    virtual ~PostprocessorBackend() {}

    /** typedef describing how the list of histograms works */
    typedef std::list<std::pair<uint64_t, HistogramBackend*> > histogramList_t;

    /** process the event
     *
     * this will evaluate the event and fill the resulting histogram. This needs
     * to be implemented in the postprocessors.
     * @param event the cassevent to work on
     */
    virtual void process(const CASSEvent& event) = 0;

    /** the operator called for each event */
    virtual const HistogramBackend& operator()(const CASSEvent& evt)
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
     * When parameter is 0 then it just returns the last known histogram.
     * @param eventid the event id of the histogram that is requested. Default is 0
     */
    virtual const HistogramBackend& getHist()(const uint64_t eventid)
    {
      using namespace std;
      QReadLocker lock(&_histLock);
      //if eventId is 0 then just return the latest event//
      if (0 == eventId)
      {
        return *_histList.back().second;
      }
      else
      {
        histogramList_t::const_iterator it
            (find_if(histList.begin(),histList.end(),IsKey(eventid)));
        if (_histList.end() == it)
          throw invalid_argument("PostProcessorBackend::getHist() : EventId is not present");
        else
          return *(it.second);
      }
    }

    /** Provide default implementation of loadSettings that does nothing */
    virtual void loadSettings(size_t)
    {
      VERBOSEOUT(std::cout << "calling backend' load settings"<<std::endl);
    }

    /** Define all postprocessors we depend on
     *
     * The dependencies must be run before the actual postprocessor is run by itself.
     */
    virtual PostProcessors::active_t dependencies()
    {
      return PostProcessors::active_t();
    }

  protected:
    /** the postprocessors key */
    PostProcessors::key_t _key;

    /** the list of histograms - event ids */
    histogramList_t _histList;

    /** pointer to the current histogram */
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

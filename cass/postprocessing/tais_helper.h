//Copyright (C) 2010 Lutz Foucar

#ifndef _TAIS_HELPER_H_
#define _TAIS_HELPER_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <list>
#include <map>
#include <iostream>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_event.h"
#include "backend.h"


namespace cass
{
  namespace Tais
  {
    /** predicate class for find_if.
     * this helps finding the right key in the list of pairs
     * @author Lutz Foucar
     */
    class IsKey
    {
    public:
      /** initialize the key in the constructor*/
      IsKey(const uint64_t key):_key(key){}
      /** compares the first element of the pair to the key*/
      bool operator()(const std::pair<uint64_t,bool>& p)const
      { return (p.first == _key); }
    private:
      /** the key that we will compare to in the operator*/
      const uint64_t _key;
    };

    /** Helper for Tais.
     * this is just a helper that we need since the whole concept is not working
     * yet.
     * @author Lutz Foucar
     */
    class TaisHelper
    {
    public:
      /** static function creating instance of this. */
      static TaisHelper * instance();

      /** destroy the whole helper*/
      static void destroy();

      /** retrieve detector for event. */
      bool condition(const CASSEvent& evt)  {return validate(evt);}

      /** tell the detector owned by this instance to reload its settings*/
      void loadSettings();

    protected:
      /** typdef defining the list of conditions for more readable code*/
      typedef std::list<std::pair<uint64_t, bool> > conditionList_t;

      /** validation of event */
      bool validate(const CASSEvent &evt)
      {
        //lock this so that only one helper will retrieve the detector at a time//
        QMutexLocker lock(&_mutex);
        //find the pair containing the detector//
        conditionList_t::iterator it =
          std::find_if(_conditionList.begin(), _conditionList.end(), IsKey(evt.id()));

//        std::cout<< "TaisHelper::validate():"
//            <<" "<<it->first
//            <<" "<<it->second
//            <<std::endl;
        //check wether id is not already on the list//
        if(_conditionList.end() == it)
        {
          //process the event and find condition
          bool cond (process(evt));
//          std::cout<< "TaisHelper::validate(): condition "<<cond<<std::endl;
          //create a new key from the id with the reloaded detector
          conditionList_t::value_type newPair = std::make_pair(evt.id(),cond);
          //put it to the beginning of the list//
          _conditionList.push_front(newPair);
          //erase the outdated element at the back//
          _conditionList.pop_back();
          //make the iterator pointing to the just added element of the list//
          it = _conditionList.begin();
        }
        return it->second;
      }

      /** process an event and find the condition */
      bool process(const CASSEvent& evt);

    protected:
      /** list of pairs of id-condition */
      conditionList_t _conditionList;

      /** the condition type */
      enum ConditionType{Tof, VMI, PnCCD, PnCCDPhotonhit} _conditiontype;

      /** tof boundaries for calc integral */
      std::pair<float,float> _tofBound;

      /** range for condition of integral */
      std::pair<float,float> _tofCond;

      /** ccd integral boundaries */
      std::pair< std::pair<float,float> , std::pair<float,float> > _ccdBound;

      /** second ccd integral boundaries */
      std::pair< std::pair<float,float> , std::pair<float,float> > _ccdBound2;

      /** range for condition of integral */
      std::pair<float,float> _ccdCond;

    private:
      /** prevent people from constructin other than using instance().*/
      TaisHelper();

      /** prevent copy-construction*/
      TaisHelper(const TaisHelper&);

      /** private desctuctor.
       * prevent destruction other than trhough destroy(),
       * delete the detector and the detectorlist for this instance
       */
      ~TaisHelper(){}

      /** prevent assingment */
      TaisHelper& operator=(const TaisHelper&);

      /** Singleton Mutex to lock write operations*/
      static QMutex _mutex;

      /** a static instance of this */
      static TaisHelper* _instance;
    };
  }

  class Histogram0DFloat;
  /** Result of TaisHelper.
   *
   * @author Lutz Foucar
   */
  class pp4000 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp4000(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp4000();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

  protected:
    /** resulting histgram */
    Histogram0DFloat *_result;
  };


}

#endif

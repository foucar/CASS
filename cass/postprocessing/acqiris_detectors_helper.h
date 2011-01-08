//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors_helper.h file contains declaration of classes that
 *                                  extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _DETECTOR_HELPER_H_
#define _DETECTOR_HELPER_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_acqiris.h"
#include "detector_backend.h"
#include "cass_event.h"
#include "detector_analyzer_backend.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** predicate class for find_if.
     *
     * this helps finding the right key in the list of pairs
     * @see HelperAcqirisDetectors::_detectorList
     *
     * @author Lutz Foucar
     */
    class IsKey
    {
    public:
      /** initialize the key in the constructor*/
      IsKey(const uint64_t key):_key(key){}

      /** compares the first element of the pair to the key*/
      bool operator()(const std::pair<uint64_t,DetectorBackend*>& p)const
      { return (p.first == _key); }

    private:
      /** the key that we will compare to in the operator*/
      const uint64_t _key;
    };

    /** Helper for Acqiris related Postprocessors.
     *
     * This class will return the requested detector, which signals are going to
     * a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
     * can call it without knowing about it.
     *
     * @todo make sure that the detectors are protected from beeing written
     *       while they are read from
     * @todo make acqiris helper to pp.
     *
     * @author Lutz Foucar
     */
    class HelperAcqirisDetectors
    {
    public:
      /** static function creating instance of this.
       *
       * create an instance of an helper for the requested detector.
       * if it doesn't exist already. Create the maps with analyzers
       *
       * @note creating the list of analyzers might be more useful inside the constuctor. But
       *       then there would be a map for each detector.. We need to change this to
       *       let the detectors calculate the requested vaules lazly in the near future
       */
      static HelperAcqirisDetectors * instance(Detectors);

      /** destroy the whole helper*/
      static void destroy();

      /** retrieve detector for event.
       * after validating that the event for this detector exists,
       * return the detector from our list
       */
      DetectorBackend * detector(const CASSEvent& evt)  {return validate(evt);}

      /** tell the detector owned by this instance to reload its settings*/
      void loadSettings(size_t=0);

    protected:
      /** typdef defining the list of detectors for more readable code*/
      typedef std::list<std::pair<uint64_t, DetectorBackend*> > detectorList_t;

      /** validation of event.
       *
       * validate whether we have already seen this event
       * if not than add a detector, that is copy constructed or
       * assigned from the detector this instance owns, to the list.
       *
       * @return the pointer to this detector
       * @param evt the cass event to validate
       */
      DetectorBackend * validate(const CASSEvent &evt)
      {
        //lock this so that only one helper will retrieve the detector at a time//
        QMutexLocker lock(&_helperMutex);
        //find the pair containing the detector//
        detectorList_t::iterator it =
          std::find_if(_detectorList.begin(), _detectorList.end(), IsKey(evt.id()));
        //check wether id is not already on the list//
        if(_detectorList.end() == it)
        {
          //retrieve a pointer to the acqiris device//
          Device *dev =
              dynamic_cast<Device*>(evt.devices().find(cass::CASSEvent::Acqiris)->second);
          //take the last element and get the the detector from it//
          DetectorBackend* det = _detectorList.back().second;
          //copy the information of our detector to this detector//
          det->clear();
          //process the detector using the data in the device
          (*det)(*dev);
          //create a new key from the id with the reloaded detector
          detectorList_t::value_type newPair = std::make_pair(evt.id(),det);
          //put it to the beginning of the list//
          _detectorList.push_front(newPair);
          //erase the outdated element at the back//
          _detectorList.pop_back();
          //make the iterator pointing to the just added element of the list//
          it = _detectorList.begin();
        }
        return it->second;
      }

    protected:
      /** list of pairs of id-detectors.
       *
       * The contents are copy constructed from the detector that this helper instance owns.
       * Needs to be at least the size of workers that can possibly call this helper simultaniously,
       * but should be shrinked if it get much bigger than the nbr of workers
       */
      detectorList_t _detectorList;

    private:
      /** prevent people from constructin other than using instance().*/
      HelperAcqirisDetectors() {}

      /** private constructor.
       *
       * create our instance of the detector depending on the detector type
       * and the list of detectors.
       */
      HelperAcqirisDetectors(Detectors);

      /** prevent copy-construction*/
      HelperAcqirisDetectors(const HelperAcqirisDetectors&);

      /** private desctuctor.
       *
       * prevent destruction other than trhough destroy(),
       * delete the detector and the detectorlist for this instance
       */
      ~HelperAcqirisDetectors();

      /** prevent assingment */
      HelperAcqirisDetectors& operator=(const HelperAcqirisDetectors&);

      /** the helperclass instances.
       *
       * the instances of this class put into map
       * one instance for each available detector
       */
      static std::map<Detectors,HelperAcqirisDetectors*> _instances;

      /** Singleton Mutex to lock write operations*/
      static QMutex _mutex;

      /** Mutex for each helper*/
      QMutex _helperMutex;
    };
  }
}


#endif

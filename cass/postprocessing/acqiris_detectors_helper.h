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
#include <list>
#include <string>
#include <map>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_acqiris.h"

namespace cass
{
  class CASSEvent;

  namespace ACQIRIS
  {
    class DetectorBackend;

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
      /** typedef describing the instances of the helper */
      typedef std::map<std::string,HelperAcqirisDetectors*> helperinstancesmap_t;

    protected:
      /** typedef defining the list of detectors for more readable code*/
      typedef std::list<std::pair<uint64_t, DetectorBackend*> > detectorList_t;

    public:
      /** static function creating instance of this.
       *
       * return the instance of the helper that is managing the detector. If the
       * helper is not yet inside the _instances map the helper instance will be
       * created and put into the _instances map.
       *
       * @return instance of the helper manaing the detector
       * @param detector key (name) of the detector to find it in the _instances map
       */
      static HelperAcqirisDetectors * instance(const helperinstancesmap_t::key_type& detector);

      /** destroy the whole helper*/
      static void destroy();

      /** retrieve detector for event
       *
       * this just calls validate(). See validate() for further information
       *
       * @return pointer to the detector that contains the data related to the
       *         requested event
       * @param evt the event whos data we need to relate to the detector.
       */
      DetectorBackend * detector(const CASSEvent& evt)  {return validate(evt);}

      /** retrieve detector
       *
       * this just retrieves the first detector from the _detectorList. Can be
       * used for chekcking the properties of the detector.
       *
       * @return const pointer to the first detector in _detectorList
       */
      const DetectorBackend* detector()const  {return _detectorList.front().second;}


      /** load the settings of the detectors in the detector list
       *
       * go through the list of detectors and tell each of the detector to load
       * its settings.
       *
       * @param i unused parameter
       */
      void loadSettings(size_t i=0);

      /** validate that this event has been associated with the detector.
       *
       * This function will lock, so that it can be consecutivly called by
       * different threads.\n
       * Check if the event is already associated with one of the detectors in
       * the detector list. If so just return the pointer to the detector that
       * is associated with this event.\n
       * If not then take the detector of the last element  in the _detectorList
       * and call its associate() member with this event. Then create a new
       * element to be put into the _detectorList, where the key of the element
       * is the id of the event and the value is the pointer to the detector,
       * that we associated with this event. Put the newly created element in
       * the beginning of the _detectorList and erase the last element.
       *
       * @return the pointer to this detector
       * @param evt the cass event to validate
       */
      DetectorBackend * validate(const CASSEvent &evt);

      /** retrieve the detector type that the helper is there for */
      DetectorType detectortype() {return _dettype;}

    protected:
      /** list of pairs of id-detectors.
       *
       * @note  Needs to be at least the size of workers that can possibly call
       *        this helper simultaniously, but should be shrinked if it gets
       *        much bigger than the number of workers.
       */
      detectorList_t _detectorList;

    private:
      /** prevent people from constructin other than using instance().*/
      HelperAcqirisDetectors() {}

      /** private constructor.
       *
       * Creates the list of detectors. The detectors are of the user chosen
       * type. The type can be chosen by the user via the .cass ini setting
       * dettype. The instance of the detectors are created
       * by DetectorBackend::instance() \n
       * The name of the detector is also the key in the instances map.
       *
       * @cassttng AcqirisDetectors/\%name\%/{DetectorType}\n
       *           Type of the detector that this helper should be managing.
       *           Default is 1 (TofDetector). Possible choises are
       *           - 0: Delayline (DelaylineDetector)
       *           - 1: Time of Flight Detector (TofDetector)
       *
       * @param detname the name of the detector
       */
      HelperAcqirisDetectors(const helperinstancesmap_t::key_type& detname);

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
      static helperinstancesmap_t _instances;

      /** Singleton Mutex to lock write operations*/
      static QMutex _mutex;

      /** Mutex for each helper*/
      QMutex _helperMutex;

      /** the type of detector that the individual helper is there for */
      DetectorType _dettype;
    };
  }
}


#endif

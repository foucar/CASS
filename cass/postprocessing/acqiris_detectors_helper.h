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
       * create an instance of an helper for the requested detector.
       * if it doesn't exist already. Create the maps with analyzers
       *
       * @note creating the list of analyzers might be more useful inside the constuctor. But
       *       then there would be a map for each detector.. We need to change this to
       *       let the detectors calculate the requested vaules lazly in the near future
       */
      static HelperAcqirisDetectors * instance(helperinstancesmap_t::key_type);

      /** destroy the whole helper*/
      static void destroy();

      /** retrieve detector for event
       *
       * after validating that the event for this detector exists, return the
       * detector from our list
       *
       * @return pointer to the detector that contains the data related to the
       *         requested event
       * @param evt the event whos data we need to relate to the detector.
       */
      DetectorBackend * detector(const CASSEvent& evt)  {return validate(evt);}

      /** tell the detector owned by this instance to reload its settings*/
      void loadSettings(size_t=0);

      /** validation of event.
       *
       * validate whether we have already seen this event
       * if not than add a detector, that is copy constructed or
       * assigned from the detector this instance owns, to the list.
       *
       * @return the pointer to this detector
       * @param evt the cass event to validate
       */
      DetectorBackend * validate(const CASSEvent &evt);

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
      HelperAcqirisDetectors(helperinstancesmap_t::key_type);

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
    };
  }
}


#endif

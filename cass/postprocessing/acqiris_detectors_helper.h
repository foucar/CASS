//Copyright (C) 2010 - 2014 Lutz Foucar

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
#include <vector>
#include <string>
#include <map>
#include <tr1/memory>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_acqiris.h"
#include "cass_event.h"

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
 * @cassttng AcqirisDetectors/\%name\%/{DetectorType}\n
 *           Type of the detector that this helper should be managing.
 *           Default is 1 (TofDetector). Possible choises are
 *           - 0: Delayline: see cass::ACQIRIS::DelaylineDetector
 *           - 1: Time of Flight Detector: see cass::ACQIRIS::TofDetector
 *
 * @todo make sure that the detectors are protected from beeing written
 *       while they are read from
 *
 * @author Lutz Foucar
 */
class HelperAcqirisDetectors
{
public:
  /** typedef a shared pointer of this */
  typedef std::tr1::shared_ptr<HelperAcqirisDetectors> shared_pointer;

  /** typedef describing the instances of the helper */
  typedef std::map<std::string,shared_pointer> helperinstancesmap_t;

  /** a shared pointer of the detector backend */
  typedef std::tr1::shared_ptr<DetectorBackend> Det_sptr;

  /** define the type of the id used */
  typedef CASSEvent::id_t id_type;

  /** defining a key - value pair for the list */
  typedef std::pair<id_type,Det_sptr> KeyDetPair_t;

  /** typedef defining the list of detectors for more readable code*/
  typedef std::vector<KeyDetPair_t> detectorList_t;

  /** define an iterator for the list */
  typedef detectorList_t::iterator iter_type;

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
  static shared_pointer instance(const helperinstancesmap_t::key_type& detector);

  /** release the detector of all helpers that is blocked for the event
   *
   * @param id the eventid that is assinged for the detector
   */
  static void releaseDetector(const id_type &id);

  /** return all known instances fo this */
  static const helperinstancesmap_t& instances();

  /** retrieve detector for event
   *
   * this just calls validate(). See validate() for further information
   *
   * @return pointer to the detector that contains the data related to the
   *         requested event
   * @param evt the event whos data we need to relate to the detector.
   */
  DetectorBackend& detector(const CASSEvent& evt)  {return validate(evt);}

  /** retrieve detector
   *
   * this just retrieves the first detector from the _detectorList. Can be
   * used for chekcking the properties of the detector.
   *
   * @return const pointer to the first detector in _detectorList
   */
  const DetectorBackend& detector()const  {return *_detectorList.front().second;}


  /** load the settings of the detectors in the detector list
   *
   * go through the list of detectors and tell each of the detector to load
   * its settings.
   *
   * @param i unused parameter
   */
  void loadSettings(size_t i=0);

  /** retrieve the detector type that the helper is there for */
  DetectorType detectortype() {return _dettype;}

protected:
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
   * @return reference to the validated detector
   * @param evt the cass event to validate
   */
  DetectorBackend& validate(const CASSEvent &evt);

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
   * @param detname the name of the detector
   */
  HelperAcqirisDetectors(const helperinstancesmap_t::key_type& detname);

  /** prevent copy-construction*/
  HelperAcqirisDetectors(const HelperAcqirisDetectors&);

  /** prevent assingment */
  HelperAcqirisDetectors& operator=(const HelperAcqirisDetectors&);

  /** find an element with a given id in the list
   *
   * @return iterator to the found element
   * @param id the id of the element
   */
  iter_type findId(const id_type &id);

  /** release the detector element in the list by settings its key (eventid) back
   *  to 0
   *
   * @param id the detector event id that should be released
   */
  void release(const id_type & id);

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

  /** the iterator to the last element returned */
  iter_type _lastEntry;
};
}//end namespace ACQIRIS
}//end namespace cass


#endif

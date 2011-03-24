//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_helper.h contains classes that extract and add
 *                               information of pixel detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _PIXEL_DETECTOR_HELPER_H_
#define _PIXEL_DETECTOR_HELPER_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <list>
#include <string>
#include <map>
#include <tr1/memory>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_acqiris.h"

namespace cass
{
  class CASSEvent;

  class PixelDetectorContainer;

  /** Helper for PixelDetector related Postprocessors.
   *
   * @author Lutz Foucar
   */
  class HelperPixelDetectors
  {
  public:
    /** typedef a shared pointer of this */
    typedef std::tr1::shared_ptr<HelperPixelDetectors> shared_pointer;

    /** typedef describing the instances of the helper */
    typedef std::map<std::string,shared_pointer> instancesmap_t;

    typedef std::tr1::shared_ptr<PixelDetectorContainer> PixDetContainer_sptr;

    /** typedef defining the list of detectors for more readable code*/
    typedef std::list<std::pair<uint64_t, PixDetContainer_sptr> > detectorList_t;

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
    static shared_pointer instance(const instancesmap_t::key_type& detector);

    /** retrieve detector for event
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
     * @return pointer to the detector that contains the data related to the
     *         requested event
     * @param evt the event whos data we need to relate to the detector.
     */
     PixDetContainer_sptr detector(const CASSEvent& evt);

    /** load the settings of the detectors in the detector list
     *
     * go through the list of detectors and tell each of the detector to load
     * its settings.
     *
     * @param i unused parameter
     */
    void loadSettings(size_t i=0);

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
    HelperPixelDetectors() {}

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
    HelperPixelDetectors(const instancesmap_t::key_type& detname);

    /** prevent copy-construction*/
    HelperPixelDetectors(const HelperPixelDetectors&);

    /** prevent assingment */
    HelperPixelDetectors& operator=(const HelperPixelDetectors&);

    /** the helperclass instances.
     *
     * the instances of this class put into map
     * one instance for each available detector
     */
    static instancesmap_t _instances;

    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;

    /** Mutex for each helper*/
    QMutex _helperMutex;
  };
}


#endif

//Copyright (C) 2011 Lutz Foucar

/**
 * @file common_data.h contains the common data for one advanced pixeldetector
 *
 * @author Lutz Foucar
 */

#ifndef _COMMON_DATA_H_
#define _COMMON_DATA_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <list>
#include <string>
#include <map>
#include <tr1/memory>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
/** Data used commonly for one AdvancedDetector
 *
 * This class hold the data for one AdvancedDetector. There can be multiple
 * instances of the AdvancedDetector which should use only one common data
 * for e.g. offsetmaps. This data is contained in this class. To ensure that
 * one AdvancedDetector will get only one instance the static function
 * CommonData::instance will return only a new instance when the name does not
 * exist yet.
 *
 * @cassttng PixelDetectors/{Name}\n
 *           Name of the Pixeldetector. See
 *           cass::pixeldetector::AdvancedDetector for more information.
 *
 * @author Lutz Foucar
 */
class CommonData
{
public:
  /** typedef a shared pointer of this */
  typedef std::tr1::shared_ptr<CommonData> shared_pointer;

  /** typedef describing the instances of the helper */
  typedef std::map<std::string,shared_pointer> instancesmap_t;

public:
  /** static function creating instance of this.
   *
   * return the instance of the common data. If the instance is not yet inside
   * the _instances map the helper instance will be created one and put into the
   * _instances map.
   *
   * @return instance of the common data
   * @param detector key (name) of the detector to find it in the _instances map
   */
  static shared_pointer instance(const instancesmap_t::key_type& detector);


  /** load the settings of the detectors in the detector list
   *
   * go through the list of detectors and tell each of the detector to load
   * its settings.
   *
   * @param s the object to load the settings for the common data from.
   */
  void loadSettings(CASSSettings &s);

  /** lock to synchronize read and write acces to the common data */
  QReadWriteLock lock;

  /** the offset map
   *
   * the offset map is the mean value of the individual pixels for given
   * number of frames
   */
  frame_t offsetMap;

  /** the noise map
   *
   * the noise map is the standart deviation of the mean value of indidual
   * pixels for a given number of frames
   */
  frame_t noiseMap;

  /** the detector mask
   *
   * the mask is a matrix with either 0 or 1 which indicate which pixels
   * shoud be omitted (1 stands for :take pixel, 0 is for don't take pixel
   */
  frame_t mask;

  /** the gain + cte map
   *
   * this is a matrix of values containing correction factors for each
   * individual pixel of the frame
   */
  frame_t gain_cteMap;

  /** the correction map
   *
   * this map contains the correction values calculated from the mask, the gain
   * and cte map. With this values the indivdual pixels will be mulitplied
   * in the HLL like processing of the frame
   */
  frame_t correctionMap;

private:
  /** prevent people from constructing other than using instance().*/
  CommonData() {}

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
  CommonData(const instancesmap_t::key_type& detname);

  /** prevent copy-construction*/
  CommonData(const CommonData&);

  /** prevent assingment */
  CommonData& operator=(const CommonData&);

  /** the helperclass instances.
   *
   * the instances of this class put into map
   * one instance for each available detector
   */
  static instancesmap_t _instances;

  /** mutex to lock the creation of an instance */
  static QMutex _mutex;
};

} //end namespace pixeldetector
} //end namespace cass


#endif

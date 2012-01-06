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
#include <tr1/functional>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
class Frame;
class MapCreatorBase;

/** Data used commonly for one AdvancedDetector
 *
 * This class hold the data for one AdvancedDetector. There can be multiple
 * instances of the AdvancedDetector which should use only one common data
 * for e.g. offsetmaps. This data is contained in this class. To ensure that
 * one AdvancedDetector will get only one instance the static function
 * CommonData::instance will return only a new instance when the name does not
 * exist yet.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{MapCreatorType}\n
 *           The type of functor that will create the maps used for correcting
 *           the frames from individual frames. Default is "none". Options are:
 *           - "none": All maps will be initialized such that the frame will
 *                     not be altered when applying them. See
 *                     cass::pixeldetector::NonAlteringMaps for details.
 *           - "fixed": The maps will be created from a fixed number of frames.
 *                      See cass::pixeldetector::FixedMaps for details.
 *           - "moving": The maps will be created from the last few frames. See
 *                       cass::pixeldetector::MovingMaps for details.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputOffsetNoiseFilename}\n
 *           The filename containing the saved noise and offset maps. Default
 *           is "darkcal_\%detectorId\%.cal".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputOffsetNoiseFiletype}\n
 *           The filetype that the values are stored in. Default is "hll".
 *           Options are:
 *           - "hll": the filetype used by the semi conductor lab.
 *           - "cass": the filetype formerly used in CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputOffsetNoiseFilename}\n
 *           The filename to store the noise, offset and mask in. This is only
 *           the prefix the current time and detector id will be appended to the
 *           name. See cass::pixeldetector::CommonData::saveMaps for more
 *           details Default is "darkcal".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputOffsetNoiseFiletype}\n
 *           The filetype that the values are stored to. Default is "hll".
 *           Options are:
 *           - "hll": the filetype used by the semi conductor lab.
 *           - "cass": the filetype formerly used in CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputCTEGainFilename}\n
 *           The filename containing the saved cte and gain values. Default
 *           is "".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputCTEGainFiletype}\n
 *           The filetype that the values are stored in. Default is "hll".
 *           Options are:
 *           - "hll": the filetype used by the semi conductor lab.
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

  /** a mask is a vector of bools */
  typedef std::vector<char> mask_t;

public:
  /** static function creating instance of this.
   *
   * return the instance of the common data used with the advanced detector with
   * the name detector. If the instance is not yet inside the _instances map the
   * helper instance will be created one and put into the _instances map. So
   * that later calls to this will return the same shared pointer.
   *
   * @return instance of the common data
   * @param detector key (name) of the detector to find it in the _instances map
   */
  static shared_pointer instance(const instancesmap_t::key_type& detector);

  /** load the settings of this common object
   *
   * See the description of this class for the list of possible variables that
   * will be loaded via this function. Once all parameters are loaded it will
   * load the Mask for this detector. See pixeldetector::createCASSMask for
   * options creating the mask. Also the correction map for this dector is
   * created once all information has been loaded. See createCorrectionMap for
   * details.
   *
   * @param s the object to load the settings for the common data from.
   */
  void loadSettings(CASSSettings &s);

  /** create the maps from the frame data with help of the functor
   *
   * There are several ways of creating the maps available. For a detailed list
   * see this classes help.
   *
   * @param frame The frame data to create the maps from
   */
  void createMaps(const Frame& frame);

  /** save maps
   *
   * save the maps to file in the user chosen fileformat.See
   * cass::pixeldetector::saveCASSOffsetFile or
   * cass::pixeldetector::saveHLLOffsetFile for details.
   *
   * first append the detector id and the current time to the user selected out
   * filename. Then save the file and create a link to it. The name of the link
   * will be called like "darkcal_\%detectorID\%.cal". In case the link exists
   * try to remove it first.
   */
  void saveMaps();

  /** lock to synchronize read and write acces to the common data */
  QReadWriteLock lock;

  /** the width of the maps */
  size_t columns;

  /** the height of the maps */
  size_t rows;

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
  mask_t mask;

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

  /** the threshold in adu for masking noisy pixels */
  pixel_t noiseThreshold;

  /** the id of the detector that contains the frames whos maps we have here */
  int32_t detectorId;

private:
  /** prevent people from constructing other than using instance().*/
  CommonData() {}

  /** private constructor.
   *
   * unused
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

private:
  /** functor to create the Maps */
  std::tr1::shared_ptr<MapCreatorBase> _mapcreator;

  /** function to write the offset maps */
  std::tr1::function<void(const std::string&,CommonData&)> _saveTo;

  /** output filename for the maps */
  std::string _outputOffsetFilename;
};

} //end namespace pixeldetector
} //end namespace cass


#endif

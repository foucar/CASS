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
 *                     cass::pixeldetector::MapCreatorBase for details.
 *           - "fixed": The maps will be created from a fixed number of frames.
 *                      See cass::pixeldetector::FixedMaps for details.
 *           - "moving": The maps will be created from the last few frames. See
 *                       cass::pixeldetector::MovingMaps for details.
 *           - "online": Uses a fast way to collect the frames and a fast and
 *                       simple way to calculate the maps.See
 *                       cass::pixeldetector::OnlineFixedCreator
 *                       for details.
 *           - "onlinecommonmode": same as online, but corrects the common mode
 *                                 from the frames when calculating the maps.See
 *                                 cass::pixeldetector::OnlineFixedCreatorCommonMode
 *                                 for details.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputOffsetNoiseFilename}\n
 *           The filename containing the saved noise and offset maps. Default
 *           is "darkcal_%detectorId%.lnk". Which is a link to the most recent
 *           darkcalibration file. If no file is found, the offset will be set to
 *           0 and the noise will be set to 4000. When the user has chosen a
 *           "NoisyPixelThreshold" lower than 4000 this will result in a completly
 *           masked frame.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputOffsetNoiseFiletype}\n
 *           The filetype that the values are stored in. Default is "hll".
 *           Options are:
 *           - "hll": the filetype used by the semi conductor lab.
 *           - "cass": the filetype formerly used in CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputOffsetNoiseFilename}\n
 *           The filename where the offset and noise values will be written to.
 *           If the name is "darkcal", the name will be build by the detector id
 *           and the current date and time when it was written. Also when writing
 *           a link to the written file will be created like this:
 *           "darkcal_%detectorID%.lnk". When the name differs from "darkcal",
 *           the values will be written only to the given filename. See
 *           cass::pixeldetector::CommonData::saveOffsetNoiseMaps for more details.
 *           Default is "darkcal".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputOffsetNoiseFiletype}\n
 *           The filetype that the noise and offset values are stored to.
 *           Default is "hll".
 *           Options are:
 *           - "hll": the filetype used by the semi conductor lab.
 *           - "cass": the filetype formerly used in CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{GainMapCreatorType}\n
 *           The type of functor that will create the gain used for correcting
 *           the frames from individual frames. Default is "none". Options are:
 *           - "none": All maps will be initialized such that the frame will
 *                     not be altered when applying them. See
 *                     cass::pixeldetector::MapCreatorBase for details.
 *           - "GainFixedADURange": gain value from the average pixelvalue within
 *                                  a given ADU range
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputGainFilename}\n
 *           The filename containing the gain (/cte) values. Default
 *           is "gain_\%detectorId\%.lnk".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{InputGainFiletype}\n
 *           The filetype that the gain (/cte) values are stored in. Default is
 *            "hll". Options are:
 *           - "hll": the filetype used by the semi conductor lab.
 *           - "cass": the filetype used by CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputGainFilename}\n
 *           The filename where the gain (/cte) values will be written to. If
 *           the name is "gain", the name will be build by the detector id and
 *           the current date and time when it was written. Also when writing
 *           a link to the written file will be created like this:
 *           "gain_\%detectorID\%.lnk". When the name differs from "gain", the
 *           values will be written only to the given filename. See
 *           cass::pixeldetector::CommonData::saveGainMap for more details.
 *           Default is "gain".
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{OutputGainFiletype}\n
 *           The filetype that the gain (/cte) values are stored in. Default is
 *            "cass". Options are:
 *           - "cass": the filetype used by CASS.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/{NoisyPixelThreshold}\n
 *           The threshold to identify noisy pixels. Will be used when creating
 *           the mask. When the noise of the pixel is higher than this value
 *           the pixel will be masked. Default is 40000.
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

  /** issue a command to the map creators of all instances
   *
   * @param command the command to issue
   */
  static void controlCalibration(const std::string& command);

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

  /** generate the maps from the frame data with help of the functors
   *
   * There are several ways of creating the maps available. For a detailed list
   * see this classes help.
   *
   * @param frame The frame data to create the maps from
   */
  void generateMaps(const Frame& frame);


  /** create the correction map
   * 
   * will create the correction map from the mask, noise and cte/gain values
   * with the help of the cass::pixeldetector::createCorrectionMap function
   *
   * the correction value for a pixel is calculated using the following formular:
   *
   * \f[
   *  corval = ctegain \times corval \times maskval \times (
   *     0, & \text{if} noise < noisethreshold ;
   *     1, & \text{otherwise})
   * \f]
   *
   * @note we do not need to lock this function since, it will be called by 
   *       the map creators only. And their operators are still locked by
   *       this classes createMaps function that will envoke the functors.
   * @todo make this a friend and protected so only functions that we allow 
   *       can call it.
   */
  void createCorMap();

  /** save offset and noise maps
   *
   * save the offset and noise calibratioin to file in the user chosen
   * fileformat. See
   * cass::pixeldetector::saveCASSOffsetFile or
   * cass::pixeldetector::saveHLLOffsetFile for details.
   *
   * If the filename is "darkcal" for the darkcalibration file, the detector id
   * and the current time will be appended the filename. A link to the created
   * files will be generated. The name of the link will be called
   * "darkcal_\%detectorID\%.lnk". In case the link exists, try to remove it
   * first.
   */
  void saveOffsetNoiseMaps();

  /** save gain map
   *
   * save the gain calibration to file in the user chosen fileformat. See
   * cass::pixeldetector::saveCASSGaiFile for details.
   *
   * If the filename is "gain" for the gain calibration file, the detector id
   * and the current time will be appended to the filename. A link
   * to the created files will be generated. The name of the link will be
   * called "gain_\%detectorID\%.lnk". In case the links exist, try to
   * remove it first.
   */
  void saveGainMap();

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
  CommonData(const CommonData&) {}

  /** prevent assingment */
  CommonData& operator=(const CommonData&) {return *this;}

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
  std::tr1::shared_ptr<MapCreatorBase> _offsetnoiseMapcreator;

  /** function to write the offset maps */
  std::tr1::function<void(const std::string&,CommonData&)> _saveNoiseOffsetTo;

  /** output filename for the offset and noise maps */
  std::string _outputOffsetFilename;

  /** input filename of the offset and noise map */
  std::string _inputOffsetFilename;

  /** switch to tell that load settins for this common data was already running */
  bool _settingsLoaded;

  /** functor to create the Maps */
  std::tr1::shared_ptr<MapCreatorBase> _gainCreator;

  /** function to write the gain map */
  std::tr1::function<void(const std::string&,CommonData&)> _saveGainTo;

  /** the gain correction input filename */
  std::string _inputGainFilename;

  /** the gain correction output filename */
  std::string _outputGainFilename;


  /** function to read the gain / cte corrections */
//  std::tr1::function<void(const std::string&,CommonData&)> _readGain;
};

} //end namespace pixeldetector
} //end namespace cass


#endif

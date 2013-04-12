// Copyright (C) 2013 Lutz Foucar

/**
 * @file gaincalibration.h contains a gain calibration functor
 *
 * @author Lutz Foucar
 */

#ifndef _GAINCALIBRATION_H_
#define _GAINCALIBRATION_H_

#include <tr1/memory>
#include <tr1/functional>
#include <vector>

#include "mapcreator_base.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;
class CommonData;


/** Creates a gain calibration
 *
 * Creates the maps after collecting a user given number of frames. The maps will
 * be created using standart statistics. The user has the choice of whehter the
 * mean or the median should be used for the offset value.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{NbrFrames}\n
 *           The number of frames that should be collected for calculating the
 *           maps. Default is 200.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{StartInstantly}\n
 *           Flag to tell whether the calculator should start instantly with
 *           collecting the frames and calculating the maps. If false it will
 *           wait until told by the program through the available GUI's. Default
 *           is false.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{DisregardedHighValues}\n
 *           Number of highest values that should be disregarded when calculating
 *           the offset value. Default is 5
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{DisregardedLowValues}\n
 *           Number of lowest values that should be disregarded when calculating
 *           the offset value. Default is 0
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{UseMedian}\n
 *           Tell the creator to use a median to calculate the offset value.
 *           If this is false the offset is calculated via the mean value.
 *           Default is false
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedCreator/{WriteMaps}\n
 *           Tell the creator to write the calulated maps once they have been
 *           calculated. For further infomration on how the files are written,
 *           see cass::pixeldetector::CommonData. Default is true.
 *
 * @author Lutz Foucar
 */
class GainCalibration : public MapCreatorBase
{
public:
  /** build map from frame
   *
   * take the input frame and use its data to build up the correction maps. But
   * only when the _createMaps flag is set to true.
   *
   * Once the _storage container is full calculate the maps. First retrieve for
   * each pixel a list of all pixels within the storage that don't contain an
   * event (e.g. photonhit).
   *
   * The map resources are locked since the function calling this operator will
   * lock the resources.
   *
   * After calculation reset the _createMaps flag and clear the storage.
   *
   * @param frame the frame containing the data to build the maps from
   */
  void operator() (const Frame &frame) {_createMap(frame);}

  /** load the settings of this creator
   *
   * See class description for a detailed list.
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

  /** start the gain calibration
   *
   * @param unused unused parameter
   */
  void controlCalibration(const std::string& unused);

private:
  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function object that will be called by the operator */
  std::tr1::function<void(const Frame&)> _createMap;

  /** a function that just returns and does nothing
   *
   * @param unused not used
   */
  void doNothing(const Frame& /*unused*/) {}

  /** generate gain calibration data
   *
   * @param frame the frames that will be added to the calibration
   */
  void generateCalibration(const Frame& frame);

//  /** the function that will calculate the offset */
//  std::tr1::function<frame_t::value_type(frame_t&, size_t, size_t)> _calcOffset;

//  /** the storage with all the frames from which the maps are calculated */
//  storage_t _storage;

//  /** flag to tell whether the maps should be created */
//  bool _createMaps;

//  /** how many frames should be included to create the statistics */
//  size_t _nbrFrames;

//  /** how many highest values should be disregarded */
//  size_t _maxDisregarded;

//  /** how many lowest values should be disregarded */
//  size_t _minDisregarded;

//  /** write maps flag */
//  bool _writeMaps;
};



} //end namespace pixeldetector
} //end namespace cass
#endif

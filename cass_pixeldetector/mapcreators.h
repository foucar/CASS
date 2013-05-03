// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreators.h contains all correction map creators.
 *
 * @author Lutz Foucar
 */

#ifndef _MAPCREATORS_H_
#define _MAPCREATORS_H_

#include <tr1/memory>
#include <tr1/functional>
#include <vector>

#include "mapcreator_base.h"
#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;
class CommonData;


/** Creates maps from a fixed number of Frames
 *
 * Creates the maps after collecting a user given number of frames. The maps will
 * be created using standart statistics. The user has the choice of whehter the
 * mean or the median should be used for the offset value.
 *
 * @MapCreateList "fixed": The maps will be created from a fixed number of frames.
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
class FixedMaps : public MapCreatorBase
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
  void operator() (const Frame &frame);

  /** load the settings of this creator
   *
   * See class description for a detailed list.
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

  /** start accumulating the maps */
  void controlCalibration(const std::string& /*unused*/) {_createMaps = true;}

private:
  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function that will calculate the offset */
  std::tr1::function<frame_t::value_type(frame_t&, size_t, size_t)> _calcOffset;

  /** the storage with all the frames from which the maps are calculated */
  storage_t _storage;

  /** flag to tell whether the maps should be created */
  bool _createMaps;

  /** how many frames should be included to create the statistics */
  size_t _nbrFrames;

  /** how many highest values should be disregarded */
  size_t _maxDisregarded;

  /** how many lowest values should be disregarded */
  size_t _minDisregarded;

  /** write maps flag */
  bool _writeMaps;
};


/** Creates maps from the last number of maps
 *
 * The algorithm that calculates the running average (offset) and standard
 * deviation is taken from here (last checked 30.01.2012):
 * http://www-uxsup.csx.cam.ac.uk/~fanf2/hermes/doc/antiforgery/stats.pdf
 *
 * @MapCreateList "moving": The maps will be created from the last few frames.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/ChangingCreator/{NbrTrainingFrames}\n
 *           The number of frames that should be collected for calculating the
 *           initial maps. Default is 50.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/ChangingCreator/{DoTraining}\n
 *           Before updating collect a set of "training" frames from which the
 *           initial maps are created. This is recommended if one does not have
 *           or has an outdated noise and offset map. Default is false.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/ChangingCreator/{NbrOfAverages}\n
 *           The number of past frames that should be taken into account when
 *           calculating the noise and offset map. Default is 50.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/ChangingCreator/{Multiplier}\n
 *           How much bigger does the pixel value have to be than the noise before
 *           The pixel is not taken into account when calculating the offset and
 *           noise of that pixel. Default is 4.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/ChangingCreator/{AutoSaveSize}\n
 *           Tells to save the noise and offset map and recalculate the correction
 *           map after this many frames. Default is 1e6.
 *
 * @author Lutz Foucar
 */
class MovingMaps : public MapCreatorBase
{
public:
  /** build map from frame
   *
   * The map resources are locked since the function calling this operator will
   * lock the resources.
   *
   * take the input frame and use its data to build up the correction maps.
   *
   * @param frame the frame containing the data to build the maps from
   */
  void operator() (const Frame &frame) {_createMap(frame);}

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

  /** start the training
   *
   * @param unused  (not used)
   */
  void controlCalibration(const std::string& unused);

private:
  /** train the maps
   *
   * collect user settable amount of frames and create intial maps from it
   * these maps will then be updated with each new frame
   *
   * @param frame the frame to build up the storage and to calc the maps from
   */
  void train(const Frame &frame);

  /** update the existing map with the incomming frame inforamtion
   *
   * will update the maps with the information of the frame
   *
   * @param frame the frame to update the maps from
   */
  void updateMaps(const Frame &frame);

private:
  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function object that will be called by the operator */
  std::tr1::function<void(const Frame&)> _createMap;

  /** the storage with all the frames from which the maps are calculated */
  storage_t _storage;

  /** counter to keep track how many frames are collected */
  size_t _framecounter;

  /** after wich number of frames should the maps be written to file */
  size_t _frameSave;

  /** how much frames should the training include */
  size_t _trainingsize;

  /** how many frames should the memory go back */
  float _alpha;

  /** how much noise is allowed before disregarding pixel from list */
  float _multiplier;
};

} //end namespace pixeldetector
} //end namespace cass
#endif

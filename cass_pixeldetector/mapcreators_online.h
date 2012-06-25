// Copyright (C) 2012 Lutz Foucar

/**
 * @file mapcreators_online.h contains correction map creators that work fast
 *                            easy for online purposes.
 *
 * @author Lutz Foucar
 */

#ifndef _MAPCREATORSONLINE_H_
#define _MAPCREATORSONLINE_H_

#include <tr1/memory>
#include <tr1/functional>
#include <vector>

#include "mapcreator_base.h"
#include "cass_pixeldetector.h"
#include "commonmode_calculator_base.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;
class CommonData;

/** Creates the maps fast and simple
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreator/{Multiplier}\n
 *           How much bigger does the pixel value have to be than the noise before
 *           The pixel is not taken into account when calculating the offset and
 *           noise of that pixel. Default is 4.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreator/{NbrFrames}\n
 *           The number of frames that should be collected for calculating the
 *           maps. Default is 200.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreator/{StartInstantly}\n
 *           Flag to tell whether the calculator should start instantly with
 *           collecting the frames and calculating the maps. If false it will
 *           wait until told by the program through the available GUI's. Default
 *           is false.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreator/{WriteMaps}\n
 *           Tell the creator to write the calulated maps once they have been
 *           calculated. For further infomration on how the files are written,
 *           see cass::pixeldetector::CommonData. Default is true.
 *
 * @author Lutz Foucar
 */
class OnlineFixedCreator : public MapCreatorBase
{
public:
  /** the operator
   *
   * just calls the function that creates the map. This function is exchanged
   * depending on whether the frames should be calculated or not.
   *
   * @param frame the frame to check for
   */
  void operator() (const Frame &frame) {_createMap(frame);}

  /** start accumulating the maps
   *
   * @param command the comamnd that the gui issued to this creator
   */
  void controlCalibration(const std::string& unused);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** the special storage type of this class */
  typedef std::vector< std::vector<pixel_t>  > specialstorage_t;

  /** a function that just returns and does nothing
   *
   * @param unused not used
   * @param unused not used
   */
  void doNothing(const Frame& /*unused*/) {}

  /** build up storage and then calculate the maps
   *
   * @param frame the frame to build up the storage and to calc the maps from
   */
  void buildAndCalc(const Frame& frame);

  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function object that will be called by the operator */
  std::tr1::function<void(const Frame&)> _createMap;

  /** storage where the pixels are already ordered */
  specialstorage_t _specialstorage;

  /** how many frames should be collected before the maps are calculated */
  size_t _nbrFrames;

  /** the multiplier to define the max noise before the pixel is considered to contain a photon */
  pixel_t _multiplier;

  /** flag wether the create maps should be saved to file or just used */
  bool _writeMaps;

  /** counter to keep track how many frames are collected */
  size_t _framecounter;
};




/** Creates the maps fast and simple with commond mode correction
 *
 * Uses collects the user chosen nbr of frames and uses them to calculate the
 * correction and noise maps. In addition it will correct the frames from the
 * common mode noise.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreatorCommonMode/{Multiplier}\n
 *           How much bigger does the pixel value have to be than the noise before
 *           The pixel is not taken into account when calculating the offset and
 *           noise of that pixel. Default is 4.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreatorCommonMode/{NbrFrames}\n
 *           The number of frames that should be collected for calculating the
 *           maps. Default is 200.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreatorCommonMode/{StartInstantly}\n
 *           Flag to tell whether the calculator should start instantly with
 *           collecting the frames and calculating the maps. If false it will
 *           wait until told by the program through the available GUI's. Default
 *           is false.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreatorCommonMode/{WriteMaps}\n
 *           Tell the creator to write the calulated maps once they have been
 *           calculated. For further infomration on how the files are written,
 *           see cass::pixeldetector::CommonData. Default is true.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/FixedOnlineCreatorCommonMode/{CommonModeCalculationType}\n
 *           The type of commonmode calculation thats used to correct the common
 *           mode when trying to get rid of it. Default is "simpleMean". Possible
 *           values are:
 *           - "none": No common mode correction is done
 *           - "simpleMean": The common mode is calculated from the mean value
 *                           Which is determined with a simple algorithm. See
 *                           cass::pixeldetector::commonmode::SimpleMeanCalculator
 *           - "mean": The common mode is calculated from the mean value of the
 *                     pixels. See cass::pixeldetector::commonmode::MeanCalculator
 *           - "median": The common mode is calculated from the median of the
 *                       pixels. See
 *                       cass::pixeldetector::commonmode::MedianCalculator for
 *                       details.
 *
 * @author Lutz
 */
class OnlineFixedCreatorCommonMode : public MapCreatorBase
{
public:
  /** the operator
   *
   * just calls the function that creates the map. This function is exchanged
   * depending on whether the frames should be calculated or not.
   *
   * @param frame the frame to check for
   */
  void operator() (const Frame &frame) {_createMap(frame);}

  /** start accumulating the maps
   *
   * @param command the comamnd that the gui issued to this creator
   */
  void controlCalibration(const std::string& unused);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:

  /** a function that just returns and does nothing
   *
   * @param unused not used
   */
  void doNothing(const Frame& /*unused*/) {}

  /** build up storage and then calculate the maps
   *
   * @param frame the frame to build up the storage and to calc the maps from
   */
  void buildAndCalc(const Frame& frame);

  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function object that will be called by the operator */
  std::tr1::function<void(const Frame&)> _createMap;

  /** storage where the pixels are already ordered */
  storage_t _storage;

  /** how many frames should be collected before the maps are calculated */
  size_t _nbrFrames;

  /** the multiplier to define the max noise before the pixel is considered to contain a photon */
  pixel_t _multiplier;

  /** flag wether the create maps should be saved to file or just used */
  bool _writeMaps;

  /** counter to keep track how many frames are collected */
  size_t _framecounter;

  /** functor for calculating the common mode level */
  commonmode::CalculatorBase::shared_pointer _commonModeCalculator;
};


} //end namespace pixeldetector
} //end namespace cass
#endif

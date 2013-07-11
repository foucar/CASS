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
#include "commonmode_calculator_base.h"

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
 * Generates the gain map.
 *
 * @GainMapCreateList "GainFixedADURange": gain value from the average pixelvalue
 *                                         within a given ADU range
 *
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{StartInstantly}\n
 *           Flag to tell whether the calculator should start instantly after
 *           loading the seetings. If false it will wait until told by the
 *           program through the available GUI's. Default is false.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{NbrFrames}\n
 *           The number of frames that should be processed before the gain map is calculated.
 *           Default is -1, which means that the only criteria is that the MinimumMedianCounts
 *           have to be reached before calulating the gain map.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{MinimumPhotonCount}\n
 *           How many times the adu value of the pixel was in the user defined
 *           ADURange, before the gain for this pixel will be calculated. Default
 *           is 50
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{MinimumMedianCounts}\n
 *           The gain values are only calculates when median counts per pixel
 *           exeed this value. Default is 50.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{MinADURange|MaxADURange}\n
 *           The range in ADU in which the pixelvalue has to lie before it is
 *           taken into the statistics. Default is 0|1000.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{SaveCalibration}\n
 *           If true writes the gain calibration to file. For further
 *           information on how the files are written, see
 *           cass::pixeldetector::CommonData. Default is true.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/GainFixedADURange/{CommonModeCalculationType}\n
 *           defines what kind of common mode correction should be done with the
 *           frames. There are settings that are used fo all calculation types.
 *           see cass::pixeldetector::commonmode::CalculatorBase. Default type
 *           is "none". Possible values are:
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
 * @author Lutz Foucar
 */
class GainCalibration : public MapCreatorBase
{
public:
  /** generate gain map from frame
   *
   * use the _createMap functor that either does nothing or generates the gain
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

  /** the value that will be set when not enough statistics is present */
  frame_t::value_type _constGain;

  /** define a conatiner for a statistics of a pixel */
  typedef std::pair<int,double> statistics_t;

  /** container for the statistics for each pixel */
  std::vector<statistics_t> _statistics;

  /** range of ADU values that are of interest */
  std::pair<frame_t::value_type,frame_t::value_type> _range;

  /** the minimum nbr of photons that the median needs before the gain will calculated */
  statistics_t::first_type _minMedianCounts;

  /** minimum nbr of photon seen by a pixel before the average will used */
  statistics_t::first_type _minPhotonCount;

  /** flag whether to write the gain calibration to file */
  bool _writeFile;

  /** functor for calculating the common mode level */
  commonmode::CalculatorBase::shared_pointer _commonModeCalculator;

  /** counter to see how many frames have been processed */
  int64_t _counter;

  /** how many frames should be processed */
  int64_t _nFrames;
};



} //end namespace pixeldetector
} //end namespace cass
#endif

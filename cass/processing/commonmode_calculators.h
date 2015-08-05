// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.h contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#ifndef _COMMONMODECALCULATORS_H_
#define _COMMONMODECALCULATORS_H_

#include <tr1/memory>
#include <vector>

#include "commonmode_calculator_base.h"

namespace cass
{
namespace pixeldetector
{

namespace commonmode
{
typedef std::vector<pixel_t> pixels_t;

/** Calculate the common mode by taking mean of pixel values
 *
 * This functor calculates the mean value of the pixels. To create the list
 * of pixels it will check whether the pixel might contain an event such as an
 * photon. If so it will not be included in the mean. There must be a minimum
 * number of pixels without an event before the common mode is calculated.
 *
 * To calculate the mean a simple accumulating average is done.
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/SimpleMeanCommonMode/{MinNbrPixels}\n
 *           The minimal number of Pixels that should be found in the range
 *           before the common mode value will be calculated. Default is 8
 *
 * @author Lutz Foucar
 */
class SimpleMeanCalculator : public CalculatorBase
{
public:
  /** constructor */
  SimpleMeanCalculator() {}

  /** the operation
   *
   * determine the common mode level and return it.
   *
   * @return the common mode level
   * @param pixel the start pixel within the frame to start the common mode
   *        calculation
   * @param idx index of the first pixel.
   */
  pixel_t operator ()(frame_t::const_iterator pixel, size_t idx)const;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** mininmal number of pixels that should be present when calculating the mean */
  size_t _minNbrPixels;
};


/** Calculate the common mode by taking mean of pixel values
 *
 * This functor calculates the mean value of the pixels. To create the list
 * of pixels it will check whether the pixel might contain an event such as an
 * photon. If so it will not be included in the mean. There must be a minimum
 * number of pixels without an event before the common mode is calculated.
 *
 * The user has the option to not include a number of highest and lowest pixel
 * values when calculating the mean. This helps to get rid of possible extremes.
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MeanCommonMode/{NbrMinDisregardedValues}\n
 *           The number of lowest values that should be disreagarded when
 *           calculating the mean value. Default is 5.
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MeanCommonMode/{NbrMaxDisregardedValues}\n
 *           The number of highest values that should be disreagarded when
 *           calculating the mean value. Default is 0.
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MeanCommonMode/{MinNbrPixels}\n
 *           The minimal number of Pixels that should be found in the range
 *           before the common mode value will be calculated. Default is 8
 *
 * @author Lutz Foucar
 */
class MeanCalculator : public CalculatorBase
{
public:
  /** constructor */
  MeanCalculator() {}

  /** the operation
   *
   * determine the common mode level and return it.
   *
   * @return the common mode level
   * @param pixel the start pixel within the frame to start the common mode
   *        calculation
   * @param idx index of the first pixel.
   */
  pixel_t operator ()(frame_t::const_iterator pixel, size_t idx)const;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** number of maximum elements to remove */
  size_t _nbrMaximumElementsToRemove;

  /** number of minimum elements to remove */
  size_t _nbrMinimumElementsToRemove;

  /** mininmal number of pixels that should be present when calculating the mean */
  int _minNbrPixels;
};


/** Calculate the common mode by taking the median of pixel values
 *
 * This functor retrieves the median value of the pixels. To create the list
 * of pixels it will check whether the pixel might contain an event such as an
 * photon. If so it will not be included in the mean. There must be a minimum
 * number of pixels without an event before the common mode is calculated.
 *
 * The user has the option to not include a number of highest and lowest pixel
 * values when getting the median of the distribution. This helps to get rid of
 * possible extremes.
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MedianCommonMode/{NbrMinDisregardedValues}\n
 *           The number of lowest values that should be disreagarded when
 *           getting the median value. Default is 5.
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MedianCommonMode/{NbrMinDisregardedValues}\n
 *           The number of highest values that should be disreagarded when
 *           getting the median value. Default is 0.
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/MedianCommonMode/{MinNbrPixels}\n
 *           The minimal number of Pixels that should be found in the range
 *           before the common mode value will be calculated. Default is 8.
 *
 * @author Lutz Foucar
 */
class MedianCalculator : public CalculatorBase
{
public:
  /** constructor */
  MedianCalculator() {}

  /** the operation
   *
   * determine the common mode level and return it.
   *
   * @return the common mode level
   * @param pixel the start pixel within the frame to start the common mode
   *        calculation
   * @param idx index of the first pixel.
   */
  pixel_t operator ()(frame_t::const_iterator pixel, size_t idx)const;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** number of maximum elements to remove */
  size_t _nbrDisregardedMaximumElements;

  /** number of minimum elements to remove */
  size_t _nbrDisregardedMinimumElements;

  /** mininmal number of pixels that should be present when calculating the mean */
  int _minNbrPixels;
};
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass

#endif

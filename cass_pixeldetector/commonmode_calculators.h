// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculators.h contains all available common mode calculators.
 *
 * @author Lutz Foucar
 */

#ifndef _COMMONMODECALCULATORS_H_
#define _COMMONMODECALCULATORS_H_

#include <tr1/memory>

#include "commonmode_calculator_base.h"

namespace cass
{
namespace pixeldetector
{

namespace commonmode
{

/** Calculate the common mode by taking mean of pixel values
 *
 * description
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/CommonModeMean/{CommonModeCalculationType}\n
 *
 * @author Lutz Foucar
 */
class MeanCalculator : public CalculatorBase
{
public:
  /** constructor */
  MeanCalculator();

  /** the operation
   *
   * determine the common mode level and return it.
   *
   * @return the common mode level
   * @param pixel iterator to the first pixel of the area used for common mode
   *              calculation.
   * @param idx index of the first pixel.
   */
  pixel_t operator ()(frame_t::iterator &pixel, size_t idx)const;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** number of maximum and minimum elements to remove */
  std::pair<size_t,size_t> _nbrElementsToRemove;

  /** mininmal number of pixels that should be present when calculating the mean */
  int _minNbrPixels;
};

/** Calculate the common mode by taking the median of pixel values
 *
 * Will calculate the common mode level by taking the median of the area of
 * pixels. User can choose how many upper and lower values should be disregarded
 * when calculating the median.
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/CommonModeMean/{CommonModeCalculationType}\n
 *
 * @author Lutz Foucar
 */
class MedianCalculator : public CalculatorBase
{
public:
  /** constructor */
  MedianCalculator();

  /** the operation
   *
   * determine the common mode level and return it.
   *
   * @return the common mode level
   * @param pixel iterator to the first pixel of the area used for common mode
   *              calculation.
   * @param idx index of the first pixel.
   */
  pixel_t operator ()(frame_t::iterator &pixel, size_t idx)const;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** how many highest values should be disregarded */
  size_t _nbrValuesDisregardedUpper;
};
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass

#endif

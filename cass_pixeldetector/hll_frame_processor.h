// Copyright (C) 2011 Lutz Foucar

/**
 * @file hll_frame_processor.h contains hll correctionlike frame processor.
 *
 * @author Lutz Foucar
 */

#ifndef _HLLFRAMEPROCESSOR_H_
#define _HLLFRAMEPROCESSOR_H_

#include "frame_processor_base.h"
#include "common_data.h"

namespace cass
{
namespace pixeldetector
{
namespace commonmode
{
class CalulatorBase;
}

/** HLL like frame processing
 *
 * this processor will do a processing of the frame that should be similar
 * to what the semi conductor lab in Munich (HLL) is doing to process the
 * pnCCD frames
 *
 * @cassttng PixelDetectors/\%name\%/HLLProcessing/{CommonModeCalculationType}\n
 *           defines what kind of common mode correction should be done with the
 *           frames. Default is "none". Possible values are:
 *           - "none": No common mode correction is done
 *           - "mean": The common mode is calculated from the mean value of the
 *                     pixels. See cass::pixeldetector::commonmode::MeanCalculator
 *           - "median": The common mode is calculated from the median of the
 *                       pixels. See
 *                       cass::pixeldetector::commonmode::MedianCalculator for
 *                       details.
 *
 * @author Lutz Foucar
 */
class HLLProcessor : public FrameProcessorBase
{
public:
  /** constructor */
  HLLProcessor();

  /** process the frame
   *
   * take the input frame, process it and then return a reference to it
   *
   * @return reference to the processed frame
   * @param frame the frame that should be processed
   */
  Frame& operator() (Frame &frame);

  /** load the settings of this processor
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** the commonly used data container */
  CommonData::shared_pointer _commondata;

  /** functor for calculating the common mode level */
  CommonModeCalulatorBase::shared_pointer _commonModeCalc;
};
}//end namespace pixeldetector
}//end namespace cass

#endif

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
class CommonModeCalulatorBase;

/** HLL like frame processing
 *
 * this processor will do a processing of the frame that should be similar
 * to what the semi conductor lab in Munich (HLL) is doing to process the
 * pnCCD frames
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

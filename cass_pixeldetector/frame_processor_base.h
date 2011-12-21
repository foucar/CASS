// Copyright (C) 2011 Lutz Foucar

/**
 * @file frame_processor_base.h contains base class for all frame processors.
 *
 * @author Lutz Foucar
 */

#ifndef _FRAMEPROCESSORBASE_H_
#define _FRAMEPROCESSORBASE_H_

#include <tr1/memory>
#include <vector>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;

/** base class for all frame processors
 *
 * a frame processor will process the pixels of a frame. For a list of possible
 * processors see description of pixeldetector::AdvancedDetector.
 *
 * @author Lutz Foucar
 */
class FrameProcessorBase
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<FrameProcessorBase> shared_pointer;

  /** virtual destructor */
  virtual ~FrameProcessorBase() {}

  /** create an instance of the requested functor
   *
   * @return a shared pointer to the requested type
   * @param type the reqested type
   */
  static shared_pointer instance(const std::string &type);

  /** process the frame
   *
   * take the input frame, process it and then return a reference to it
   *
   * @return reference to the processed frame
   * @param frame the frame that should be processed
   */
  virtual Frame& operator() (Frame &frame)=0;

  /** load the settings of this processor
   *
   * @param s the CASSSettings object to read the information from
   */
  virtual void loadSettings(CASSSettings &s)=0;
};

} //end namespace pixeldetector
} //end namespace cass
#endif

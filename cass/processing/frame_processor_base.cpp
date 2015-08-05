// Copyright (C) 2011 Lutz Foucar

/**
 * @file frame_processor_base.cpp  contains base class for all frame processors
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "frame_processor_base.h"

#include "hll_frame_processor.h"


using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

namespace cass
{
namespace pixeldetector
{
/** Class with no processing
 *
 * this functor will just return the frame and leave it unprocessed
 *
 * @author Lutz Foucar
 */
class NoProcess : public FrameProcessorBase
{
public:
  /** just returns the frame */
  Frame& operator ()(Frame& frame) {return frame;}

  /** no settings to be loaded */
  void loadSettings(CASSSettings&) {}
};
}
}

FrameProcessorBase::shared_pointer FrameProcessorBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "none")
    ptr = shared_pointer(new NoProcess());
  else if (type == "hll")
    ptr = shared_pointer(new HLLProcessor());
  else
    throw invalid_argument("FrameProcessorBase::instance: Frame processor type '"+
                           type + "' is unknown.");
  return ptr;
}

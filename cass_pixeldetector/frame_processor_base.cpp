// Copyright (C) 2011 Lutz Foucar

/**
 * @file frame_processor_base.cpp  contains base class for all frame processors
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "frame_processor_base.h"


using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

namespace cass
{
namespace pixeldetector
{
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
//  else if (type == "hll")
//    ptr = shared_pointer(new HLLProcess());
  else
  {
    stringstream ss;
    ss << "FrameProcessorBase::instance: Frame processor type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

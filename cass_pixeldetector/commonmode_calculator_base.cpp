// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculator_base.cpp contains base class for all common mode
 *                                      calculators.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "commonmode_calculator_base.h"


using namespace cass;
using namespace pixeldetector;
using namespace commonmode;
using namespace std;
using namespace std::tr1;

namespace cass
{
namespace pixeldetector
{
namespace commonmode
{
/** Just returns a constant 0.
 *
 * this functor will just return 0 with any calculation with effectively will
 * make sure that no common mode correction will be done:
 *
 * @author Lutz Foucar
 */
class NoCalc : public CalculatorBase
{
public:
  /** the operation
   *
   * @return 0.
   * @param pixel unused
   * @param idx unused
   */
  pixel_t& operator ()(frame_t::const_iterator &pixel, size_t idx) {return 0;}

  /** no settings to be loaded */
  void loadSettings(CASSSettings&) {}
};
}
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
  {
    stringstream ss;
    ss << "FrameProcessorBase::instance: Frame processor type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculator_base.cpp contains base class for all common mode
 *                                      calculators.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "commonmode_calculator_base.h"

#include "commonmode_calculators.h"
#include "common_data.h"
#include "cass_settings.h"

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
  /** no operation performed
   *
   * @return 0.
   * @param pixel unused
   * @param idx unused
   */
  pixel_t operator ()(frame_t::iterator &/*pixel*/, size_t /*idx*/)const {return 0;}

  /** need to load the settings of the base class loaded */
  void loadSettings(CASSSettings& s) {load(s);}
};
}//end namespace commonmode
}//end namespace pixeldetector
}//end namespace cass

CalculatorBase::shared_pointer CalculatorBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "none")
    ptr = shared_pointer(new NoCalc());
  else if (type == "mean")
    ptr = shared_pointer(new MeanCalculator());
  else if (type == "median")
    ptr = shared_pointer(new MedianCalculator());
  else
  {
    /** @todo get rid of stringstream */
    stringstream ss;
    ss << "CalculatorBase::instance: Common Mode Calculator type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

void CalculatorBase::load(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  _commondata = CommonData::instance(detectorname);
  s.beginGroup("CommonModeCorrection");
  _nbrPixels = s.value("Width",128).toUInt();
  s.endGroup();
}

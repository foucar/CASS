// Copyright (C) 2011 Lutz Foucar

/**
 * @file commonmode_calculator_base.h contains base class for all common mode
 *                                    calculators.
 *
 * @author Lutz Foucar
 */

#ifndef _COMMONMODECALCULATORBASE_H_
#define _COMMONMODECALCULATORBASE_H_

#include <tr1/memory>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;

namespace commonmode
{
/** base class for all common mode calculators
 *
 * the calculators determine the common mode of the a user defined part of the
 * row and return the value for correcting the frame.
 *
 * @author Lutz Foucar
 */
class CalculatorBase
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<CalculatorBase> shared_pointer;

  /** virtual destructor */
  virtual ~CalculatorBase() {}

  /** create an instance of the requested functor
   *
   * @return a shared pointer to the requested type
   * @param type the reqested type
   */
  static shared_pointer instance(const std::string &type);

  /** determine the common mode value
   *
   * take the input pixel and calculate the common mode value
   *
   * @return the common mode value
   * @param pixel the pixel iterator the first pixel of the area to calculate
   *              common mode.
   * @param idx index where the pixel iterator is right now within the frame
   */
  virtual pixel_t operator() (frame_t::const_iterator &pixel, size_t idx)=0;

  /** load the settings of this calculator
   *
   * @param s the CASSSettings object to read the information from
   */
  virtual void loadSettings(CASSSettings &s)=0;
};

} //end namespace commonmode
} //end namespace pixeldetector
} //end namespace cass
#endif

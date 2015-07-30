// Copyright (C) 2010-2013 Lutz Foucar

#ifndef _IMAGING_H_
#define _IMAGING_H_

#include "processor.h"

namespace cass
{
//forward declaration
class CASSEvent;

/** Test image
 *
 * @PPList "240": Test image
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{sizeX} \n
 *           Width of testimage (default = 1024)
 * @cassttng Processor/\%name\%/{sizeY} \n
 *           Height of testimage (default = 1024)
 * @cassttng Processor/\%name\%/{FixedValue} \n
 *           Use a fixed value instead of the product of the column and row index.
 *           Default is false
 * @cassttng Processor/\%name\%/{Value} \n
 *           In case FixedValue is true, this is the value that the image will
 *           be filled with. Default is 0
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp240 : public Processor
{
public:
  /** constructor. */
  pp240(const name_t&);

  /** overwrite default behaviour and just return the constant */
  virtual const HistogramBackend& result(const CASSEvent::id_t)
  {
    return *_result;
  }

  /** overwrite default behaviour don't do anything */
  virtual void releaseEvent(const CASSEvent &){}

  /** overwrite default behaviour don't do anything */
  virtual void processEvent(const CASSEvent&){}

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

private:
  /** the constant iamge */
  std::tr1::shared_ptr<Histogram2DFloat> _result;

};



}
#endif

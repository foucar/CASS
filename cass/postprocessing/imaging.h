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
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{sizeX} \n
 *           Width of testimage (default = 1024)
 * @cassttng PostProcessor/\%name\%/{sizeY} \n
 *           Height of testimage (default = 1024)
 *
 * @author Stephan Kassemeyer
 */
class pp240 : public PostProcessor
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

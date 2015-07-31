// Copyright (C) 2010 Stephan Kassemeyer

/** @file rankfilter.h file contains processors that will operate
 *                     on histograms of other processors, calculating
 *                     statistical rank filters like median filter.
 * @author Stephan Kassemeyer
 */

#ifndef _RANKFILTER_H_
#define _RANKFILTER_H_

#include "processor.h"
#include "cass_event.h"
#include "histogram.h"
#include <deque>


namespace cass
{
/** calculate median of last values.
 *
 * @PPList "301": calculate median of last values
 *
 * If input histogram is > 0d, its values get
 *  summed up prior to median calculation.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the processor name that contain the first histogram. Default
 *           is 0.
 * @cassttng Processor/\%name\%/{MedianSize} \n
 *           how many last values should be included in median calculation.
 *           default is 100.
 *
 * @todo make more general: operate on bins. now operates on sum.
 * @author Stephan Kassemeyer.
 * @author Lutz Foucar
 */
class pp301 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp301(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &res);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first histogram */
  shared_pointer _one;

  /** last N items to be used for median calculation */
  unsigned int _medianSize;

  /** storage of last values for median calculation */
  std::deque<float> _medianStorage;

  /** mutex to lock the median storage */
  QMutex _mutex;
};






/** load data from binary dump into 2DHistogram
 *
 * @PPList "302": load data from binary dump into 2DHistogram
 *
 *  example python code to generate binary file:
 *
 *  import numpy
 *  arr = numpy.zeros( (1024,1024), dtype=numpy.float32 )
 *  cord = numpy.mgrid[0:1024,0:1024]
 *  arr[ cord[0]**2+cord[1]**2 < 5000 ] = 1
 *  arr.tofile('waterJetMask.bin')
 *
 *  import vigra
 *  import numpy
 *  arr = vigra.readImage('waterJetMask.png')
 *  arr = numpy.array( arr[:,:,0], dtype=numpy.float32 )  # only save one color channel
 *  arr /= 255     # rgb -> 0..1
 *  arr = arr.T    # transpose image to match cass coordinate system
 *  arr.tofile('waterJetMask_png.bin')
 *
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{BinaryFile} \n
 *           Filename of binary float[sizeX*sizeY] file which contains data.
 *           Default is "".
 *
 * @cassttng Processor/\%name\%/{SizeX} \n
 *           nr of Pixels for x axis.
 *           Default is 0.
 *
 * @cassttng Processor/\%name\%/{SizeY} \n
 *           nr of Pixels for y axis.
 *           Default is 0.
 *
 * @author Stephan Kassemeyer
 */
class pp302 : public Processor
{
public:
  /** constructor */
  pp302(const name_t&);

  /** overwrite default behaviour and just return the constant */
  virtual const HistogramBackend& result(const CASSEvent::id_t)
  {
    return *_result;
  }

  /** overwrite and do nothing */
  virtual void processEvent(const CASSEvent&){}

  /** overwrite default behaviour don't do anything */
  virtual void releaseEvent(const CASSEvent &){}

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

private:
  /** the constant result */
  std::tr1::shared_ptr<Histogram2DFloat> _result;
};

}//end namespace cass

#endif

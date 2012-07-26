// Copyright (C) 2010 Stephan Kassemeyer

/** @file rankfilter.h file contains postprocessors that will operate
 *                     on histograms of other postprocessors, calculating
 *                     statistical rank filters like median filter.
 * @author Stephan Kassemeyer
 */

#ifndef _RANKFILTER_H_
#define _RANKFILTER_H_

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"
#include <deque>




namespace cass
{


  /** calculate median of last values. If input histogram is > 0d, its values get
   *  summed up prior to median calculation.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{MedianSize} \n
   *           how many last values should be included in median calculation.
   *           default is 100.
   *
   * @todo make more general: operate on bins. now operates on sum.
   * @author Stephan Kassemeyer.
   */
  class pp301 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp301(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** last N items to be used for median calculation */
    unsigned int _medianSize;

    /** storage of last values for median calculation */
    std::deque<float> *_medianStorage;
  };






  /** load data from binary dump into 2DHistogram
   *
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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{binaryFile} \n
   *           Filename of binary float[sizeX*sizeY] file which contains data.
   *           Default is "".
   *
   * @cassttng PostProcessor/\%name\%/{sizeX} \n
   *           nr of Pixels for x axis.
   *           Default is 0.
   *
   * @cassttng PostProcessor/\%name\%/{sizeY} \n
   *           nr of Pixels for y axis.
   *           Default is 0.
   *
   * @author Stephan Kassemeyer
   */
  class pp302 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp302(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);
  };











}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

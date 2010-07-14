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
   * @cassttng PostProcessor/\%name\%/{medianSize} \n
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









}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:

// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains postprocessors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#ifndef _IMAGEMANIPULATION_H__
#define _IMAGEMANIPULATION_H__

#include <utility>
#include <tr1/functional>

#include "backend.h"

namespace cass
{
/** rotate, transpose, invert axis on 2d histogram.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the first histogram. Default
 *           is "".
 *
 * @author Lutz Foucar
 */
class pp55 : public PostprocessorBackend
{
public:
  /** constructor
   *
   * @param pp the postprocessor manager that manages this pp
   * @param key the name of this postprocessor in the ini file
   */
  pp55(PostProcessors& pp, const PostProcessors::key_t& key);

  /** process event
   *
   * @param evt the event to process
   */
  virtual void process(const CASSEvent& evt);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t /*unused*/);

  /** notification that depandant histogram has changed
   *
   * change own histograms when one of the ones we depend on has changed
   * histograms
   *
   * @param hist the changed histogram that we take the size from
   */
  virtual void histogramsChanged(const HistogramBackend* hist);

protected:
  /** pp containing 2d histogram */
  PostprocessorBackend *_one;

  /** the size of the original histogram */
  std::pair<size_t,size_t> _size;

  /** function that will calc the corresponding bin */
  std::tr1::function<size_t(size_t, size_t, const std::pair<size_t,size_t>&)> _pixIdx;
};

}//end namspace cass
#endif

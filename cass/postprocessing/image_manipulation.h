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
#include <string>
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
 * @cassttng PostProcessor/\%name\%/{Operation} \n
 *           the operation that one wants to perform on the 2d histogram.
 *           Default is "90DegCCW". Possible values are:
 *           - "90DegCCW" or "270DegCW": rotate the 2d hist by 90 deg counter
 *                                       clock wise bzw. 270 deg clock wise
 *           - "270DegCCW" or "90DegCW": rotate the 2d hist by 270 deg counter
 *                                       clock wise bzw. 90 deg clock wise
 *           - "180Deg": rotate the 2d hist by 180 deg
 *           - "Transpose": transpose x and y axis of the 2d hist
 *           - "FlipVertical": flip the 2d hist vertically
 *           - "FlipHorizontal": flip the 2d hist horizontally
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
  /** functions to calc the index of source from the indizes of the destination */
  typedef std::tr1::function<size_t(size_t, size_t, const std::pair<size_t,size_t>&)> func_t;

  /** pp containing 2d histogram */
  PostprocessorBackend *_one;

  /** the size of the original histogram */
  std::pair<size_t,size_t> _size;

  /** function that will calc the corresponding bin */
  func_t _pixIdx;

  /** container for all functions */
  std::map<std::string, std::pair<func_t,bool> > _functions;

  /** the user chosen operation */
  std::string _operation;
};






/** convert cspad 2d histogram into cheetah layout
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 *
 * @author Lutz Foucar
 */
class pp1600 : public PostprocessorBackend
{
public:
  /** constructor
   *
   * @param pp the postprocessor manager that manages this pp
   * @param key the name of this postprocessor in the ini file
   */
  pp1600(PostProcessors& pp, const PostProcessors::key_t& key);

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

protected:
  /** pp containing 2d histogram */
  PostprocessorBackend *_one;

  /** nbr bins in x of asic */
  const size_t _nx;

  /** nbr bins in y of asic */
  const size_t _ny;

  /** magic cheetah number */
  const size_t _na;
};





/** convert cspad 2d histogram into a quasi oriented layout
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 *
 * @author Lutz Foucar
 */
class pp1601 : public PostprocessorBackend
{
public:
  /** constructor
   *
   * @param pp the postprocessor manager that manages this pp
   * @param key the name of this postprocessor in the ini file
   */
  pp1601(PostProcessors& pp, const PostProcessors::key_t& key);

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

protected:
  /** pp containing 2d histogram */
  PostprocessorBackend *_one;

  /** nbr bins in x of asic */
  const size_t _nx;

  /** nbr bins in y of asic */
  const size_t _ny;
};

}//end namspace cass
#endif

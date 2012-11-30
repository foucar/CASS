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
//forward declarations
class SegmentCopier;

/** matrix rotation struct
 *
 * tell how much one has to advance in the destination matrix when one advances
 * by one in the src matrix
 *
 * @author Lutz Foucar
 */
struct Rotor
{
  /** contructor
   *
   * intializes the struct in the proper way
   *
   * @param cc steps to increase in the dest columns when src column is increased by one
   * @param cr steps to increase in the dest columns when src row is increased by one
   * @param rc steps to increase in the dest row when src column is increased by one
   * @param rr steps to increase in the dest row when src row is increased by one
   */
  Rotor(char cc, char cr, char rc, char rr)
    : incDestColPerSrcCol(cc),
      incDestColPerSrcRow(cr),
      incDestRowPerSrcCol(rc),
      incDestRowPerSrcRow(rr)
  {}

  /** steps to increase in the dest columns when src column is increased by one*/
  char incDestColPerSrcCol;

  /** steps to increase in the dest columns when src row is increased by one*/
  char incDestColPerSrcRow;

  /** steps to increase in the dest row when src column is increased by one*/
  char incDestRowPerSrcCol;

  /** steps to increase in the dest row when src row is increased by one*/
  char incDestRowPerSrcRow;
};

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
 * Converts the cass representation of the CsPad where all segments are in a
 * linearized matrix on top of each other where the origin is in the lower left
 * corner like follows (see also cass::pixeldetector::Converter)
@verbatim
  +-------------+
  |     31      |
  +-------------+
  |     30      |
  +-------------+
  |     29      |
  +-------------+
        .
        .
        .
  +-------------+
  |     02      |
  +-------------+
^ |     01      |
| +-------------+
y |     00      |
| +-------------+
+---x--->
@endverbatim
 * These segments have to be rearranged to get the quasi physical layout of the
 * CsPad. Following the description given in ElementIterator.hh. One has to
 * arrange the segments as follows. The arrows denote the origin of the segment
 * in the source. The x axis of the source follows always the longer edge of the
 * rectangular segment shape. The y axis of the source matrix the shorter edge.
@verbatim
    <---+    <---+ +--->                      <---+    <---+
  +----+|  +----+| |+-------------+         +----+|  +----+|  +-------------+
  |    |v  |    |v v|     06      |         |    |v  |    |V  |     13      |^
  |    |   |    |   +-------------+         |    |   |    |   +-------------+|
  |    |   |    |                           |    |   |    |              <---+
  | 05 |   | 04 |                           | 11 |   | 10 |
  |    |   |    |  +--->                    |    |   |    |
  |    |   |    |  |+-------------+         |    |   |    |   +-------------+
  |    |   |    |  V|     07      |         |    |   |    |   |     12      |^
  +----+   +----+   +-------------+         +----+   +----+   +-------------+|
                                                                         <---+
            quadrant 0                                quadrant 1
 +--->                                     +--->                <---+    <---+
 |+------------ +   +----+   +----+        |+------------ +   +----+|  +----+|
 v|     02      |   |    |   |    |        v|     08      |   |    |v  |    |v
  +-------------+   |    |   |    |         +-------------+   |    |   |    |
                    |    |   |    |                           |    |   |    |
                    | 00 |   | 01 |                           | 15 |   | 14 |
 +--->              |    |   |    |        +--->              |    |   |    |
 |+-------------+   |    |   |    |        |+-------------+   |    |   |    |
 v|     03      |  ^|    |  ^|    |        v|     09      |   |    |   |    |
  +-------------+  |+----+  |+----+         +-------------+   +----+   +----+
                   +--->    +--->




                                              <---+    <---+
  +----+   +----+   +-------------+         +----+|  +----+|  +-------------+
  |    |   |    |   |     25      |^        |    |v  |    |v  |     19      |^
  |    |   |    |   +-------------+|        |    |   |    |   +-------------+|
  |    |   |    |              <---+        |    |   |    |              <---+
  | 30 |   | 31 |                           | 17 |   | 16 |
  |    |   |    |                           |    |   |    |
  |    |   |    |   +-------------+         |    |   |    |   +-------------+
 ^|    |  ^|    |   |     24      |^        |    |   |    |   |     18      |^
 |+----+  |+----+   +-------------+|        +----+   +----+   +-------------+|
 +--->    +--->                <---+                                     <---+
            quadrant 3                                quadrant 2
 +--->
 |+------------ +   +----+   +----+         +------------ +   +----+   +----+
 v|     28      |   |    |   |    |         |     23      |^  |    |   |    |
  +-------------+   |    |   |    |         +-------------+|  |    |   |    |
                    |    |   |    |                    <---+  |    |   |    |
                    | 26 |   | 27 |                           | 20 |   | 21 |
 +--->              |    |   |    |                           |    |   |    |
 |+-------------+   |    |   |    |         +-------------+   |    |   |    |
 v|     29      |  ^|    |  ^|    |         |     22      |^ ^|    |  ^|    |
  +-------------+  |+----+  |+----+         +-------------+| |+----+  |+----+
                   +--->    +--->                     <---+ +--->    +--->
@endverbatim
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

private:
  std::tr1::shared_ptr<SegmentCopier> _copyMatrixSegment;
  /** the rotation matrix if x of src goes from (L)eft to (R)ight in dest and
   *  y of the src goes from (T)op to (B)ottom in destination
   */
  const Rotor _LRTB;

  /** the rotation matrix if x of src goes from (R)ight to (L)eft in dest and
   *  y of the src goes from (B)ottom to (T)op in destination
   */
  const Rotor _RLBT;

  /** the rotation matrix if x of src goes from (T)op to (B)ottom in dest and
   *  y of the src goes from (R)ight to (L)eft in destination
   */
  const Rotor _TBRL;

  /** the rotation matrix if x of src goes from (B)ottom to (T)op in dest and
   *  y of the src goes from (L)eft to (R)ight in destination
   */
  const Rotor _BTLR;

  /** nbr bins in x of asic */
  const size_t _nx;

  /** nbr bins in y of asic */
  const size_t _ny;
};

}//end namspace cass
#endif

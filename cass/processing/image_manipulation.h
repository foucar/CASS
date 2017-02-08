// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains processors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#ifndef _IMAGEMANIPULATION_H__
#define _IMAGEMANIPULATION_H__

#include <utility>
#include <string>
#include <tr1/functional>

#include "processor.h"

#include "geom_parser.h"

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





/** rotate, transpose, invert axis on 2d result.
 *
 * @PPList "55": rotate, transpose or invert axis on 2d result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName} \n
 *           the processor name that contain the first histogram. Default
 *           is "".
 * @cassttng Processor/\%name\%/{Operation} \n
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
class pp55 : public Processor
{
public:
  /** constructor */
  pp55(const name_t &);

  /** process event
   *
   * @param evt the event to process
   * @param result the histogram where the result will be written to
   */
  virtual void process(const CASSEvent& evt, result_t&);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t /*unused*/);

protected:
  /** functions to calc the index of source from the indizes of the destination */
  typedef std::tr1::function<size_t(size_t, size_t, const std::pair<size_t,size_t>&)> func_t;

  /** pp containing 2d histogram */
  shared_pointer _one;

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
 * @PPList "1600": convert cass to cheetah layout
 *
 * The CASS layout is just every segement on top of each other as follows:
 *
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
 * wheras the Cheetah layout puts the segemtns of the quadrant into one column as
 * follows:
@verbatim
  +-------------+-------------+-------------+-------------+
  |     07      |     15      |     23      |     31      |
  +-------------+-------------+-------------+-------------+
  |     06      |     14      |     22      |     30      |
  +-------------+-------------+-------------+-------------+
  |     05      |     13      |     21      |     29      |
  +-------------+-------------+-------------+-------------+
  |     04      |     12      |     20      |     28      |
  +-------------+-------------+-------------+-------------+
  |     03      |     11      |     19      |     27      |
  +-------------+-------------+-------------+-------------+
  |     02      |     10      |     18      |     26      |
  +-------------+-------------+-------------+-------------+
^ |     01      |     09      |     17      |     25      |
| +-------------+-------------+-------------+-------------+
y |     00      |     08      |     16      |     24      |
| +-------------+-------------+-------------+-------------+
+---x--->
@endverbatim
 *
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName} \n
 *           the processor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 *
 * @author Lutz Foucar
 */
class pp1600 : public Processor
{
public:
  /** constructor */
  pp1600(const name_t &);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t unused);

protected:
  /** pp containing 2d histogram */
  shared_pointer _one;

  /** nbr bins in x of asic */
  const size_t _nx;

  /** nbr bins in y of asic */
  const size_t _ny;

  /** magic cheetah number */
  const size_t _na;
};





/** convert cspad 2d histogram into a quasi oriented layout
 *
 * @PPList "12": Constant Value
 *
 * Converts the cass representation of the CsPad where all segments are in a
 * linearized matrix on top of each other where the origin is in the lower left
 * corner like follows (see also cass::pixeldetector::Converter). There are
 * 2*194 pixels along the x-axis and 185 pixels along y in one segment.
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
 * With this one looks at the detector from upstream, thus having a left handed
 * coordinate system since increasing x values go from left to right, where in
 * an right handed coordinates system it should go from right to left.
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


                                       X

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
                   +--->    +--->                      <---+ +--->    +--->
@endverbatim
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName} \n
 *           the processor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 *
 * @author Lutz Foucar
 */
class pp1601 : public Processor
{
public:
  /** constructor */
  pp1601(const name_t &);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t unused);

protected:
  /** pp containing 2d histogram */
  shared_pointer _one;

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










/** convert cspad data into laboratory frame using crystfel geometry files
 *
 * @PPList "1602": convert cspad data into labframe with geom file
 *
 * generates a lookup table of where in the result image will go which pixel
 *
 * to generate a right handed corrdinate system one has to arrange the tiles
 * that are in the CASS layout as follows:
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
 * to the following layout. The gaps in between the different segments are non
 * equi distant!
@verbatim
                   +--->    +--->                      <---+ +--->    +--->
  +------------ +  |+----+  |+----+         +------------ +| |+----+  |+----+
 ^|     13      |  V|    |  V|    |         |     06      |V V|    |  V|    |
 |+-------------+   |    |   |    |         +-------------+   |    |   |    |
 +--->              |    |   |    |                           |    |   |    |
                    | 10 |   | 11 |                           | 04 |   | 05 |
                    |    |   |    |                    <---+  |    |   |    |
  +-------------+   |    |   |    |         +-------------+|  |    |   |    |
 ^|     12      |   |    |   |    |         |     07      |V  |    |   |    |
 |+-------------+   +----+   +----+         +-------------+   +----+   +----+
 +--->
            quadrant 1                                quadrant 0
 +--->    +--->                <---+                                     <---+
 |+----+  |+----+   +-------------+|        +----+   +----+   +-------------+|
 V|    |  V|    |   |     08      |V        |    |   |    |   |     02      |V
  |    |   |    |   +-------------+         |    |   |    |   +-------------+
  |    |   |    |                           |    |   |    |
  | 14 |   | 15 |                           | 01 |   | 00 |
  |    |   |    |              <---+        |    |   |    |              <---+
  |    |   |    |   +-------------+|        |    |   |    |   +-------------+|
  |    |   |    |   |     09      |V        |    |^  |    |^  |     03      |V
  +----+   +----+   +-------------+         +----+|  +----+|  +-------------+
                                              <---+    <---+

                                       .

                   +--->    +--->
  +------------ +  |+----+  |+----+         +------------ +   +----+   +----+
 ^|     19      |  V|    |  V|    |        ^|     25      |   |    |   |    |
 |+-------------+   |    |   |    |        |+-------------+   |    |   |    |
 +--->              |    |   |    |        +--->              |    |   |    |
                    | 16 |   | 17 |                           | 31 |   | 30 |
                    |    |   |    |                           |    |   |    |
  +-------------+   |    |   |    |         +-------------+   |    |   |    |
 ^|     18      |   |    |   |    |        ^|     24      |   |    |^  |    |^
 |+-------------+   +----+   +----+        |+-------------+   +----+|  +----+|
 +--->                                     +--->                <---+    <---+
            quadrant 2                                quadrant 3
                                                                         <---+
  +----+   +----+   +-------------+         +----+   +----+   +-------------+|
  |    |   |    |  ^|     23      |         |    |   |    |   |     28      |V
  |    |   |    |  |+-------------+         |    |   |    |   +-------------+
  |    |   |    |  +--->                    |    |   |    |
  | 21 |   | 20 |                           | 27 |   | 26 |
  |    |   |    |                           |    |   |    |              <---+
  |    |   |    |   +-------------+         |    |   |    |   +-------------+|
  |    |^  |    |^ ^|     22      |         |    |^  |    |^  |     29      |V
  +----+|  +----+| |+-------------+         +----+|  +----+|  +-------------+
    <---+    <---+ +--->                      <---+    <---+
@endverbatim
 * One can assume that the geom file doesn't translate from the cass layout shown
 * above, but from the cheetah layout (see cass::pp1600). In this case one has
 * to set the ConvertCheetahToCASSLayout to true (default)
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName} \n
 *           the processor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 * @cassttng Processor/\%name\%/{GeometryFilename} \n
 * @cassttng Processor/\%name\%/{ConvertCheetahToCASSLayout} \n
 * @cassttng Processor/\%name\%/{BackgroundValue} \n
 *
 * @author Lutz Foucar
 */
class pp1602 : public Processor
{
public:
  /** constructor */
  pp1602(const name_t &);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t unused);

protected:
  /** generate the lookup table by parsing the geom file */
  void setup(const result_t &srcImageHist);

  /** pp containing 2d histogram */
  shared_pointer _imagePP;

  /** the lookup table */
  GeometryInfo::lookupTable_t _lookupTable;

  /** flag whether to convert the positions in the src from cheetah to cass layout */
  bool _convertCheetahToCASSLayout;

  /** the value with wich the background should be filled */
  float _backgroundValue;

  /** filename of the geometry file */
  std::string _filename;
};











/** Create a radial average of q values from a raw detector image
 *
 * @PPList "90": Radial average from detector image using geom
 *
 * calculate the Q value for each Pixel using the geom file.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName} \n
 *           the processor name that contains the histogram containing the
 *           cspad image in cass layout. Default is "".
 * @cassttng Processor/\%name\%/{GeometryFilename} \n
 *           The geom file to use. Default is "cspad.geom".
 * @cassttng Processor/\%name\%/{ConvertCheetahToCASSLayout} \n
 *           Set this true if the geom file is for a cheetah layout of the data,
 *           but the image in ImageName is the image in CASS layout.
 * @cassttng Processor/\%name\%/{Wavelength_A} \n
 *           The wavelength in Angstroem. Can also be the name of a PP that
 *           contains the Wavelength. Default is 1.
 * @cassttng Processor/\%name\%/{DetectorDistance_m} \n
 *           The detector distance in m. Can also be the name of a PP that
 *           contains the detector distance. Default is 60e-2.
 * @cassttng Processor/\%name\%/{PixelSize_m} \n
 *           The pixel size in m. Default is 110e-6
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1D histogram
 * @cassttng Processor/\%name\%/{BadPixelValue}\n
 *           value of the bad pixel in the image. Default is 0
 * @cassttng Processor/\%name\%/{Output}\n
 *           One can choose the type of radial average that one wants to have.
 *           Default is "Q". Possible values are:
 *           - "Q": the q-averge in the defintion of q that SAXS people are
 *                  using. See pp90::Q for details.
 *           - "Resolution": the radial average in resultion values as defined
 *                           by the crystallographers. See pp90::R for details.
 *           - "Radius": the radial average in radii, where the Pixelsize_m
 *                       parameter is used to determine the radius. If one wants
 *                       to have the radius in pixel units, one just has to set
 *                       the PixelSize_m parameter to 1.
 *
 * @author Lutz Foucar
 */
class pp90 : public Processor
{
public:
  /** constructor */
  pp90(const name_t &);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

  /** load the settings of this pp
   *
   * @param unused this parameter is not used
   */
  virtual void loadSettings(size_t unused);

protected:
  /** define the table row of a single output */
  struct temp_t
  {
    temp_t(): weight(0), fill(0),bin(0){}
    result_t::value_t weight;
    result_t::value_t fill;
    size_t bin;
  };

  /** define the matrix */
  typedef std::vector<temp_t> tempArray_t;

protected:
  /** retrieve the constant wavelength
   *
   * @param id unused
   */
  double wlFromConstant(const CASSEvent::id_t&) {return _wavelength;}

  /** retrieve the wavelength from the processor
   *
   * @param id the id of the event to get the wavelength from
   */
  double wlFromProcessor(const CASSEvent::id_t& id);

  /** retrieve the constant detector distance
   *
   * @param id unused
   */
  double ddFromConstant(const CASSEvent::id_t&) {return _detdist;}

  /** retrieve the detector distance from the processor
   *
   * @param id the id of the event to get the detector distance from
   */
  double ddFromProcessor(const CASSEvent::id_t& id);

  /** get the bin when the q-value of the pixel is asked
   *
   * the q-value will be calculated using formula
   * \f[
   * Q = \frac{4\pi}{\lambda} \sin\left(\frac{1}{2}\arctan\left(\frac{r}{d}\right)\right)
   * \f]
   * with \f$\lambda\f$ being the wavelength in \f$\AA\f$, \f$r\f$ the radius
   * of the pixel and \f$d\f$ the detector distance.
   *
   * @return the struct that tell the bin, value, fill combination
   * @param pixval the value of the pixel
   * @param pixpos the position of the pixel in m
   */
  temp_t Q(const CASSEvent::id_t& id, const result_t::value_t& pixval,
           const GeometryInfo::pos_t& pixpos);

  /** get the bin when the resolution value of the pixel is asked
   *
   * the resultion will be calculated as follows
   * \f[
   * R = \frac{1}{\frac{2}{\lambda}\sin\left(\frac{1}{2}\arctan\left(\frac{r}{d}\right)\right)}
   * \f]
   * with \f$\lambda\f$ being the wavelength in \f$\AA\f$, \f$r\f$ the radius
   * of the pixel and \f$d\f$ the detector distance.
   *
   * @return the struct that tell the bin, value, fill combination
   * @param pixval the value of the pixel
   * @param pixpos the position of the pixel in m
   */
  temp_t R(const CASSEvent::id_t& id, const result_t::value_t& pixval,
           const GeometryInfo::pos_t& pixpos);

  /** get the bin when the radius of the pixel is asked
   *
   *
   * @return the struct that tell the bin, value, fill combination
   * @param pixval the value of the pixel
   * @param pixpos the position of the pixel in m
   */
  temp_t Rad(const CASSEvent::id_t& id, const result_t::value_t& pixval,
             const GeometryInfo::pos_t& pixpos);

protected:
  /** pp containing 2d histogram */
  shared_pointer _imagePP;

  /** define the normalization factors */
  typedef std::vector<result_t::value_t> normfactors_t;

  /** flag whether to convert the positions in the src from cheetah to cass layout */
  bool _convertCheetahToCASSLayout;

  /** filename of the geometry file */
  std::string _filename;

  /** the pixel positions */
  GeometryInfo::conversion_t _pixPositions_m;

  /** the wavelength in case its fixed */
  double _wavelength;

  /** pp containing wavelength in case its not fixed */
  shared_pointer _wavelengthPP;

  /** function that gets the wavelength */
  std::tr1::function<double(const CASSEvent::id_t&)> _getWavelength;

  /** the detector distance in case its fixed */
  double _detdist;

  /** pp containing detector distance in case its not fixed */
  shared_pointer _detdistPP;

  /** function that gets the detectordistance */
  std::tr1::function<double(const CASSEvent::id_t&)> _getDetectorDistance;

  /** the size of one pixel in m */
  double _np_m;

  /** value of the bad pixels */
  float _badPixVal;

  /** function to map the pixel to  histogram bin, value and fill flag */
  std::tr1::function<temp_t(const CASSEvent::id_t&,const result_t::value_t&,
                            const GeometryInfo::pos_t&)> _getBin;

  /** the axis of the result */
  result_t::axe_t _axis;
};



}//end namspace cass
#endif

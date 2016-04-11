// Copyright (C) 2012, 2013 Lutz Foucar

/**
 * @file image_manipulation.cpp file contains processors that will manipulate
   *                            2d histograms
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <tr1/functional>

#include <QtCore/QString>

#include "image_manipulation.h"

#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;

namespace cass
{
/** convert the index for rows and cols into the index of linareized array
 *
 * @return the linearized index
 * @param col the Column index of the element
 * @param row the Row index of the element
 * @param nCols the number of columns in the matrix
 */
size_t toLinearized(size_t col, size_t row, size_t nCols)
{
  return (nCols*row + col);
}

/** calculate the corresponding indezes for 90 deg ccw rotation
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t Rotate90DegCCW(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nDestCols(size.first);
  const size_t nSrcCols(size.second);
  const size_t srcCol(destRow);
  const size_t srcRow(nDestCols - destCol - 1);
  return toLinearized(srcCol,srcRow,nSrcCols);
}

/** calculate the corresponding indezes for 180 deg rotation
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t Rotate180Deg(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nDestCols(size.first);
  const size_t nDestRows(size.second);
  const size_t nSrcCols(size.first);
  const size_t srcCol(nDestCols - destCol - 1);
  const size_t srcRow(nDestRows - destRow - 1);
  return toLinearized(srcCol,srcRow,nSrcCols);
}

/** calculate the corresponding indezes for 270 deg ccw (90 cw) rotation
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t Rotate270DegCCW(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nDestRows(size.second);
  const size_t nSrcCols(size.first);
  const size_t srcCol(nDestRows - destRow - 1);
  const size_t srcRow(destCol);
  return toLinearized(srcCol,srcRow,nSrcCols);
}

/** transpose the indizes
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t Transpose(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nSrcCols(size.second);
  const size_t srcCol(destRow);
  const size_t srcRow(destCol);
  return toLinearized(srcCol,srcRow,nSrcCols);
}

/** flip matrix horizontally
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t FlipHorizontal(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nSrcCols(size.first);
  const size_t nDestRows(size.second);
  const size_t srcCol(destCol);
  const size_t srcRow(nDestRows - destRow - 1);
  return toLinearized(srcCol,srcRow,nSrcCols);
}

/** flip matrix vertically
 *
 * convert the calculated indizes into the index of the linearized matrix and
 * return it
 *
 * @return index of the src as linearized index
 * @param destCol the column index of the destination matrix
 * @param destRow the row index of the destination matrix
 * @param size the size of the destination matrix
 *
 * @author Lutz Foucar
 */
size_t FlipVertical(size_t destCol, size_t destRow, pair<size_t,size_t> size)
{
  const size_t nSrcCols(size.first);
  const size_t nDestCols(size.first);
  const size_t srcCol(nDestCols - destCol -1);
  const size_t srcRow(destRow);
  return toLinearized(srcCol,srcRow,nSrcCols);
}


/** copy from a source matrix to a destination matrix in user wanted way
 *
 * a functor that will copy segments of a source matrix into the dest matrix in
 * defined a orientation.
 *
 * @author Lutz Foucar
 */
class SegmentCopier
{
public:
  /** contructor
   *
   * sets up the boundaries for the src and dest matrices
   *
   * @param srcCols the number of colums in the src matrix
   * @param srcRows the number of rows in the src matrix
   * @param destCols the number of columns in the dest matrix.
   */
  SegmentCopier(const int srcCols, const int srcRows, const int destCols)
    : _srcCols(srcCols),
      _srcRows(srcRows),
      _destCols(destCols)
  {}

  /** copy the selected segment of the src matrix to the destination matrix
   *
   * @param src iterator to the beginning of the linearized source matrix
   * @param dest reference to the beginning of the linearized destination matrix
   * @param segment the index of the segment to be copied
   * @param destColStart dest column index where the src segment starts
   * @param destRowStart dest row index where the src segment starts
   * @param rot refernce to the rotor element that tells how the src segement is
   *            oriented in the dest matrix.
   */
  void operator()(Processor::result_t::const_iterator src,
                  Processor::result_t::iterator dest,
                  const int segment,
                  const int destColStart, const int destRowStart,
                  const Rotor &rot) const
  {
    int destRow = destRowStart;
    int destCol = destColStart;

    const int srcRowStart(segment*_srcRows);
    const int srcRowStop((segment+1)*_srcRows);
    for (int srcRow = srcRowStart; srcRow < srcRowStop; ++srcRow)
    {
      destCol = (((destCol - destColStart)) % _srcCols) + destColStart;
      destRow = (((destRow - destRowStart)) % _srcCols) + destRowStart;
      for (int srcCol = 0; srcCol < _srcCols; ++srcCol)
      {
        dest[destRow*_destCols + destCol] = src[srcRow*_srcCols + srcCol];
        destCol += rot.incDestColPerSrcCol;
        destRow += rot.incDestRowPerSrcCol;
      }
      destCol += rot.incDestColPerSrcRow;
      destRow += rot.incDestRowPerSrcRow;
    }
  }

private:
  /** the number of colums in the src matrix */
  const int _srcCols;

  /** the number of rows that one segement in the src matrix consists of */
  const int _srcRows;

  /** the number of columns in the dest matrix */
  const int _destCols;
};


}//end namespace cass



pp55::pp55(const name_t &name)
  : Processor(name)
{
  _functions["90DegCCW"] = make_pair(&cass::Rotate90DegCCW,true);
  _functions["270DegCW"] = make_pair(&cass::Rotate90DegCCW,true);
  _functions["180Deg"] = make_pair(&cass::Rotate180Deg,false);
  _functions["270DegCCW"] = make_pair(&cass::Rotate270DegCCW,true);
  _functions["90DegCW"] = make_pair(&cass::Rotate270DegCCW,true);
  _functions["Transpose"] = make_pair(&cass::Transpose,true);
  _functions["FlipVertical"] = make_pair(&cass::FlipVertical,true);
  _functions["FlipHorizontal"] = make_pair(&cass::FlipHorizontal,true);
  loadSettings(0);
}

void pp55::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("ImageName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;
  _operation = s.value("Operation","90DegCCW").toString().toStdString();
  if (_functions.find(_operation) == _functions.end())
    throw invalid_argument("pp55 (" + name() +"): Operation '" + _operation +
                           "' is not supported.");
  _pixIdx = _functions[_operation].first;
  if (_functions[_operation].second)
  {
    const result_t::axe_t& xaxis(_one->result().axis(result_t::xAxis));
    const result_t::axe_t& yaxis(_one->result().axis(result_t::yAxis));
    _size = make_pair(_one->result().shape().second,
                      _one->result().shape().first);
    createHistList (result_t::shared_pointer (new result_t(yaxis,xaxis)));
  }
  else
  {
    createHistList(_one->result().clone());
    _size = _one->result().shape();
  }

  Log::add(Log::INFO,"Processor '" +  name() + "' will do '" + _operation +
           "' on Histogram in Processor '" +  _one->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp55::process(const CASSEvent &evt, result_t &result)
{
  const result_t &src(_one->result(evt.id()));
  QReadLocker lock(&src.lock);

  result_t::iterator dest(result.begin());

  for (size_t row(0); row < _size.second; ++row)
    for (size_t col(0); col < _size.first; ++col)
      *dest++ = src[_pixIdx(col,row,_size)];
}





// --------------convert cspad 2 cheetah--------------------

pp1600::pp1600(const name_t &name)
  : Processor(name),
    _nx(194),
    _ny(185),
    _na(8)
{
  loadSettings(0);
}

void pp1600::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("ImageName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  createHistList
      (result_t::shared_pointer
        (new result_t(_na*_nx,_na*_ny)));

  Log::add(Log::INFO,"Processor '" +  name() + "' will convert Histogram in " +
           "Processor '" +  _one->name() + " into the format that cheetah is using."
           ". Condition is '" + _condition->name() + "'");
}

void pp1600::process(const CASSEvent &evt, result_t &dest)
{
  // Get the input histogram
  const result_t &src(_one->result(evt.id()));
  QReadLocker lock(&src.lock);

  const size_t pix_per_quad(8*_ny*2*_nx);
  for(size_t quadrant=0; quadrant<4; quadrant++)
  {
    for(size_t k=0; k < pix_per_quad; k++)
    {
      const size_t i = k % (2*_nx) + quadrant*(2*_nx);
      const size_t j = k / (2*_nx);
      const size_t ii  = i+(_na*_nx)*j;
      dest[ii] = src[quadrant * pix_per_quad + k];
    }
  }
}






// --------------convert cspad 2 quasi laboratory --------------------


pp1601::pp1601(const name_t &name)
  : Processor(name),
    _LRTB( 1, 0, 0,-1),
    _RLBT(-1, 0, 0, 1),
    _TBRL( 0,-1,-1, 0),
    _BTLR( 0, 1, 1, 0),
    _nx(194),
    _ny(185)
{
  loadSettings(0);
}

void pp1601::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("ImageName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  createHistList
      (result_t::shared_pointer
        (new result_t(2*(2*_nx+2*_ny),2*(2*_nx+2*_ny))));

  _copyMatrixSegment =
      std::tr1::shared_ptr<SegmentCopier>
      (new SegmentCopier(2*_nx, _ny, 2*(2*_nx+2*_ny)));

  Log::add(Log::INFO,"Processor '" +  name() + "' will convert cspad image in " +
           "Processor '" +  _one->name() + " into a condensed real layout, " +
           " looking from upstream."
           ". Condition is '" + _condition->name() + "'");
}

void pp1601::process(const CASSEvent &evt, result_t &dest)
{
  // Get the input histogram
  const result_t &src(_one->result(evt.id()));
  QReadLocker lock(&src.lock);


//  const size_t pix_per_seg(2*_nx*_ny);
//  const size_t pix_per_quad(8*pix_per_seg);

//  Rotor LRBT={ 1, 0, 0, 1};
//  Rotor LRTB={ 1, 0, 0,-1};
//  Rotor RLBT={-1, 0, 0, 1};
//  Rotor RLTB={-1, 0, 0,-1};

//  Rotor TBRL={ 0,-1,-1, 0};
//  Rotor TBLR={ 0, 1,-1, 0};
//  Rotor BTRL={ 0,-1, 1, 0};
//  Rotor BTLR={ 0, 1, 1, 0};

  const SegmentCopier& copySegment(*_copyMatrixSegment);

  //q0//
  copySegment(src.begin(),dest.begin(), 0, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(), 0, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(), 1, 2*_nx+1*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(), 2, 0*_nx+0*_ny    , 2*_nx+4*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(), 3, 0*_nx+0*_ny    , 2*_nx+3*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(), 4, 0*_nx+2*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(), 5, 0*_nx+1*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(), 6, 0*_nx+2*_ny    , 4*_nx+4*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(), 7, 0*_nx+2*_ny    , 4*_nx+3*_ny -1 ,_LRTB);

  //q1//
  copySegment(src.begin(),dest.begin(), 8, 2*_nx+2*_ny    , 2*_nx+4*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(), 9, 2*_nx+2*_ny    , 2*_nx+3*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(),10, 2*_nx+4*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(),11, 2*_nx+3*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(),12, 4*_nx+4*_ny -1 , 4*_nx+2*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),13, 4*_nx+4*_ny -1 , 4*_nx+3*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),14, 4*_nx+4*_ny -1 , 4*_nx+2*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(),15, 4*_nx+3*_ny -1 , 4*_nx+2*_ny -1 ,_TBRL);

  //q2//
  copySegment(src.begin(),dest.begin(),16, 2*_nx+4*_ny -1 , 2*_nx+2*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(),17, 2*_nx+3*_ny -1 , 2*_nx+2*_ny -1 ,_TBRL);
  copySegment(src.begin(),dest.begin(),18, 4*_nx+4*_ny -1 , 2*_nx+0*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),19, 4*_nx+4*_ny -1 , 2*_nx+1*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),20, 4*_nx+2*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(),21, 4*_nx+3*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(),22, 4*_nx+2*_ny -1 , 0*_nx+0*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),23, 4*_nx+2*_ny -1 , 0*_nx+1*_ny    ,_RLBT);

  //q3//
  copySegment(src.begin(),dest.begin(),24, 2*_nx+2*_ny -1 , 2*_nx+0*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),25, 2*_nx+2*_ny -1 , 2*_nx+1*_ny    ,_RLBT);
  copySegment(src.begin(),dest.begin(),26, 2*_nx+0*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(),27, 2*_nx+1*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(),28, 0*_nx+0*_ny    , 0*_nx+2*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(),29, 0*_nx+0*_ny    , 0*_nx+1*_ny -1 ,_LRTB);
  copySegment(src.begin(),dest.begin(),30, 0*_nx+0*_ny    , 0*_nx+2*_ny    ,_BTLR);
  copySegment(src.begin(),dest.begin(),31, 0*_nx+1*_ny    , 0*_nx+2*_ny    ,_BTLR);
}



// --------------convert cspad 2 laboratory --------------------

pp1602::pp1602(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp1602::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _imagePP = setupDependency("ImageName");
  bool ret (setupCondition());
  if (!(_imagePP && ret)) return;

  _filename = s.value("GeometryFilename","cspad.geom").toString().toStdString();
  _convertCheetahToCASSLayout = s.value("ConvertCheetahToCASSLayout",true).toBool();
  _backgroundValue = s.value("BackgroundValue",0).toFloat();

  setup(_imagePP->result());

}

void pp1602::setup(const result_t &srcImageHist)
{
  _lookupTable =
      GeometryInfo::generateLookupTable(_filename,
                                        srcImageHist.size(),
                                        srcImageHist.shape().first,
                                        _convertCheetahToCASSLayout);
  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(_lookupTable.nCols,_lookupTable.min.x,_lookupTable.max.x,"Rows"),
          result_t::axe_t(_lookupTable.nRows,_lookupTable.min.y,_lookupTable.max.y,"Cols"))));

  Log::add(Log::INFO,"Processor '" +  name() + "' will convert Histogram in " +
           "Processor '" +  _imagePP->name() + " into lab frame" +
           ". Geometry Filename '" + _filename + "'"
           ". convert from cheetah to cass '" + (_convertCheetahToCASSLayout?"true":"false") + "'"
           ". Beam center is '" + toString(-_lookupTable.min.x) + " x " + toString(-_lookupTable.min.y) + "' pixels"+
           ". Condition is '" + _condition->name() + "'");
}

void pp1602::process(const CASSEvent &evt, result_t &destImage)
{
  /** Get the input histogram and its memory */
  const result_t &srcImage(_imagePP->result(evt.id()));
  QReadLocker lock(&srcImage.lock);

  /** fill the result with the background value */
  fill(destImage.begin(),destImage.end(),_backgroundValue);

  /** iterate through the src image and put its pixels at the location in the
   *  destination that is directed in the lookup table
   */
  result_t::const_iterator srcpixel(srcImage.begin());
  result_t::const_iterator srcImageEnd(srcImage.end()-8);

  vector<size_t>::const_iterator idx(_lookupTable.lut.begin());

  for (; srcpixel != srcImageEnd; ++srcpixel, ++idx)
    destImage[*idx] = *srcpixel;

  /** relfect that only 1 event was processed and release resources */
}










//************ radial average of Q values from det image ***************

pp90::pp90(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp90::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _imagePP = setupDependency("ImageName");
  bool ret (setupCondition());
  /** use fixed value for wavelength if value can be converted to double,
   *  otherwise use the wavelength from the processor
   */
  bool isDouble(false);
  QString wlkey("Wavelength_A");
  QString wlparam(s.value(wlkey,"1").toString());
  double wlval(wlparam.toDouble(&isDouble));
  if (isDouble)
  {
    _wavelength = wlval;
    _getWavelength = bind(&pp90::wlFromConstant,this,_1);
  }
  else
  {
    _wavelengthPP = setupDependency(wlkey.toStdString());
    ret = _wavelengthPP && ret;
    _getWavelength = bind(&pp90::wlFromProcessor,this,_1);
  }
  /** use fixed value for detector distance if value can be converted to double,
   *  otherwise use the detector distance from the processor
   */
  isDouble = false;
  QString ddkey("DetectorDistance_m");
  QString ddparam(s.value(ddkey,"60e-2").toString());
  double ddval(ddparam.toDouble(&isDouble));
  if (isDouble)
  {
    _detdist = ddval;
    _getDetectorDistance = bind(&pp90::ddFromConstant,this,_1);
  }
  else
  {
    _detdistPP = setupDependency(ddkey.toStdString());
    ret = _detdistPP && ret;
    _getDetectorDistance = bind(&pp90::ddFromProcessor,this,_1);
  }
  if (!(_imagePP && ret)) return;

  _filename = s.value("GeometryFilename","cspad.geom").toString().toStdString();
  _convertCheetahToCASSLayout = s.value("ConvertCheetahToCASSLayout",true).toBool();
  _np_m = s.value("PixelSize_m",110.e-6).toDouble();
  _badPixVal = s.value("BadPixelValue",0.f).toFloat();

  const result_t &srcImageHist(_imagePP->result());
  GeometryInfo::conversion_t src2lab
      (GeometryInfo::generateConversionMap
       (_filename, srcImageHist.size(), srcImageHist.shape().first,
       _convertCheetahToCASSLayout));
  _src2labradius.resize(src2lab.size());
  for (size_t i=0; i < src2lab.size(); ++i)
    _src2labradius[i] = sqrt(src2lab[i].x * src2lab[i].x + src2lab[i].y * src2lab[i].y);

  /** create the output histogram the storage where to put the normfactors*/
  createHistList(set1DHist(name()));

  Log::add(Log::INFO,"Processor '" +  name() + "' will generate Q average from Histogram in " +
           "Processor '" +  _imagePP->name() +
           ". Geometry Filename '" + _filename + "'"
           ". convert from cheetah to cass '" + (_convertCheetahToCASSLayout?"true":"false") + "'"
           ". Wavelength in Angstroem '" + (s.value("Wavelength_A").canConvert<double>() ?
                                              toString(_wavelength) : ("from PP " + _wavelengthPP->name())) +
           "' Detector Distance in m '" + (s.value("DetectorDistance_m").canConvert<double>() ?
                                              toString(_detdist) : ("from PP " + _detdistPP->name())) +
           "' Pixel Size in um '" + toString(_np_m) +
           ". Condition is '" + _condition->name() + "'");
}

double pp90::wlFromProcessor(const CASSEvent::id_t& id)
{
  const result_t &wavelength(_wavelengthPP->result(id));
  QReadLocker lock(&wavelength.lock);
  return wavelength.getValue();
}

double pp90::ddFromProcessor(const CASSEvent::id_t& id)
{
  const result_t &detdist(_detdistPP->result(id));
  QReadLocker lock(&detdist.lock);
  return detdist.getValue();
}

struct savedivides : std::binary_function<double,double,double>
{
  double operator()(const double x, const double y)const
  {
    double retval = x/y;
    if (!std::isfinite(retval))
      retval = 0;
    return retval;
  }
};

void pp90::process(const CASSEvent &evt, result_t &result)
{
  using tr1::tuple;
  using tr1::get;

  /** Get the input histogram and its memory */
  const result_t &srcImage(_imagePP->result(evt.id()));
  QReadLocker lock(&(srcImage.lock));

  /** iterate through the src image and put its pixels at the correct position
   *  in the vector containing the radial average only when they are not masked
   */
  normfactors_t normfactors(result.size(),0);
  size_t ImageSize(_src2labradius.size());
  const double lambda(_getWavelength(evt.id()));
  const double D(_getDetectorDistance(evt.id()));
  if (fuzzyIsNull(lambda) || fuzzyIsNull(D))
    return;
  const double firstFactor(4.*3.1415/lambda);
  vector<tuple<size_t,float,int> >  tmparr(_src2labradius.size());
#ifdef _OPENMP
#pragma omp for
#endif
  for (size_t i=0; i<ImageSize; ++i)
  {
    const double Q(firstFactor * sin(0.5*atan(_src2labradius[i]*_np_m/D)));
    const size_t bin(histogramming::bin(result.axis(result_t::xAxis),Q));
    get<0>(tmparr[i]) = bin;
    if(fuzzycompare(srcImage[i],_badPixVal))
    {
      get<1>(tmparr[i]) = 0;
      get<2>(tmparr[i]) = 0;
    }
    else
    {
      get<1>(tmparr[i]) = srcImage[i];
      get<2>(tmparr[i]) = 1;
    }
  }

  for (size_t i(0); i<ImageSize; ++i)
  {
    result[get<0>(tmparr[i])] += get<1>(tmparr[i]);
    normfactors[get<0>(tmparr[i])] += get<2>(tmparr[i]);
  }

  /** normalize by the number of fills for each bin */
  transform(result.begin(),result.end(),normfactors.begin(),result.begin(),savedivides());
}

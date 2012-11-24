// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains postprocessors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#include <QtCore/QString>

#include "image_manipulation.h"

#include "cass_settings.h"
#include "histogram.h"
#include "log.h"

using namespace cass;
using namespace std;


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

struct Rotor
{
  char incDestColPerSrcCol;
  char incDestColPerSrcRow;
  char incDestRowPerSrcCol;
  char incDestRowPerSrcRow;
};

/** copy from a source matrix to a destination matrix in user wanted way
 *
 * details
 *
 * @param
 *
 * @author Lutz Foucar
 */
class MatrixCopier
{
public:

  MatrixCopier(const HistogramFloatBase::storage_t& src, HistogramFloatBase::storage_t& dest,
               const int srcCols, const int srcRows,
               const int destCols)
    : _src(src),
      _dest(dest),
      _srcCols(srcCols),
      _srcRows(srcRows),
      _destCols(destCols)
  {}

  void copyMatrixSegment(const int segment,
                         const int destColStart, const int destRowStart,
                         const Rotor &rot)
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
        _dest[destRow*_destCols + destCol] = _src[srcRow*_srcCols + srcCol];
        destCol += rot.incDestColPerSrcCol;
        destRow += rot.incDestRowPerSrcCol;
      }
      destCol += rot.incDestColPerSrcRow;
      destRow += rot.incDestRowPerSrcRow;
    }
  }

private:
  const HistogramFloatBase::storage_t& _src;
  HistogramFloatBase::storage_t& _dest;
  const int _srcCols;
  const int _srcRows;
  const int _destCols;
};


}//end namespace cass



pp55::pp55(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
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
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;
  _operation = s.value("Operation","90DegCCW").toString().toStdString();
  if (_functions.find(_operation) == _functions.end())
    throw invalid_argument("pp55 (" + _key +"): Operation '" + _operation +
                           "' is not supported.");
  _pixIdx = _functions[_operation].first;
  if (_functions[_operation].second)
  {
    const AxisProperty& xaxis(_one->getHist(0).axis()[HistogramBackend::xAxis]);
    const AxisProperty& yaxis(_one->getHist(0).axis()[HistogramBackend::yAxis]);
    _result = new Histogram2DFloat(yaxis.nbrBins(),yaxis.lowerLimit(),yaxis.upperLimit(),
                                   xaxis.nbrBins(),xaxis.lowerLimit(),xaxis.upperLimit(),
                                   xaxis.title(),yaxis.title());

  }
  else
  {
    _result = _one->getHist(0).clone();
  }
  createHistList(2*NbrOfWorkers);
  _size = make_pair(_result->axis()[HistogramBackend::xAxis].nbrBins(),
                    _result->axis()[HistogramBackend::yAxis].nbrBins());

  Log::add(Log::INFO,"PostProcessor '" +  _key + "' will do '" + _operation +
           "' on Histogram in PostProcessor '" +  _one->key() +
           "'. Condition is '" + _condition->key() + "'");
}

void pp55::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  if (_functions[_operation].second)
  {
    const AxisProperty& xaxis(_one->getHist(0).axis()[HistogramBackend::xAxis]);
    const AxisProperty& yaxis(_one->getHist(0).axis()[HistogramBackend::yAxis]);
    _result = new Histogram2DFloat(yaxis.nbrBins(),yaxis.lowerLimit(),yaxis.upperLimit(),
                                   xaxis.nbrBins(),xaxis.lowerLimit(),xaxis.upperLimit(),
                                   xaxis.title(),yaxis.title());

  }
  else
  {
    _result = _one->getHist(0).clone();
  }
  createHistList(2*NbrOfWorkers);
  _size = make_pair(_result->axis()[HistogramBackend::xAxis].nbrBins(),
                    _result->axis()[HistogramBackend::yAxis].nbrBins());
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp55::process(const CASSEvent &evt)
{
  // Get the input histogram
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>((*_one)(evt)));

  hist.lock.lockForRead();
  const HistogramFloatBase::storage_t& src(hist.memory()) ;
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t::iterator dest(
        dynamic_cast<HistogramFloatBase*>(_result)->memory().begin());
  _result->nbrOfFills()=1;
  for (size_t row(0); row < _size.second; ++row)
    for (size_t col(0); col < _size.first; ++col)
      *dest++ = src[_pixIdx(col,row,_size)];
  _result->lock.unlock();
  hist.lock.unlock();
}







pp1600::pp1600(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key),
    _nx(194),
    _ny(185),
    _na(8)
{
  loadSettings(0);
}

void pp1600::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = new Histogram2DFloat(_na*_nx,_na*_ny);

  createHistList(2*NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" +  _key + "' will convert Histogram in " +
           "PostProcessor '" +  _one->key() + " into the format that cheetah is using."
           ". Condition is '" + _condition->key() + "'");
}

void pp1600::process(const CASSEvent &evt)
{
  // Get the input histogram
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>((*_one)(evt)));

  hist.lock.lockForRead();
  const HistogramFloatBase::storage_t& src(hist.memory()) ;
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t& dest(
        dynamic_cast<HistogramFloatBase*>(_result)->memory());
  _result->nbrOfFills()=1;

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
  _result->lock.unlock();
  hist.lock.unlock();
}








pp1601::pp1601(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key),
    _nx(194),
    _ny(185)
{
  loadSettings(0);
}

void pp1601::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = new Histogram2DFloat(2*(2*_nx+2*_ny),2*(2*_nx+2*_ny));

  createHistList(2*NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" +  _key + "' will convert cspad image in " +
           "PostProcessor '" +  _one->key() + " into a condensed real layout, " +
           " looking from upstream."
           ". Condition is '" + _condition->key() + "'");
}

void pp1601::process(const CASSEvent &evt)
{
  // Get the input histogram
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>((*_one)(evt)));

  hist.lock.lockForRead();
  const HistogramFloatBase::storage_t& src(hist.memory()) ;
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t& dest(dynamic_cast<HistogramFloatBase*>(_result)->memory());
  _result->nbrOfFills()=1;

  const size_t pix_per_seg(2*_nx*_ny);
  const size_t pix_per_quad(8*pix_per_seg);

  Rotor LRBT{ 1, 0, 0, 1};
  Rotor LRTB{ 1, 0, 0,-1};
  Rotor RLBT{-1, 0, 0, 1};
  Rotor RLTB{-1, 0, 0,-1};

  Rotor TBRL{ 0,-1,-1, 0};
  Rotor TBLR{ 0, 1,-1, 0};
  Rotor BTRL{ 0,-1, 1, 0};
  Rotor BTLR{ 0, 1, 1, 0};

  MatrixCopier mc(src,dest,hist.axis()[HistogramBackend::xAxis].nbrBins(),
                  hist.axis()[HistogramBackend::yAxis].nbrBins(),
                  _result->axis()[HistogramBackend::xAxis].nbrBins());
  //q0//
  mc.copyMatrixSegment( 0, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,BTLR);
  mc.copyMatrixSegment( 1, 2*_nx+1*_ny    , 2*_nx+2*_ny    ,BTLR);
  mc.copyMatrixSegment( 2, 0*_nx+0*_ny    , 2*_nx+4*_ny -1 ,LRTB);
  mc.copyMatrixSegment( 3, 0*_nx+0*_ny    , 2*_nx+3*_ny -1 ,LRTB);
  mc.copyMatrixSegment( 4, 0*_nx+2*_ny -1 , 4*_nx+4*_ny -1 ,TBRL);
  mc.copyMatrixSegment( 5, 0*_nx+1*_ny -1 , 4*_nx+4*_ny -1 ,TBRL);
  mc.copyMatrixSegment( 6, 0*_nx+2*_ny    , 4*_nx+4*_ny -1 ,LRTB);
  mc.copyMatrixSegment( 7, 0*_nx+2*_ny    , 4*_nx+3*_ny -1 ,LRTB);

  //q1//
  mc.copyMatrixSegment( 8, 2*_nx+2*_ny    , 2*_nx+4*_ny -1 ,LRTB);
  mc.copyMatrixSegment( 9, 2*_nx+2*_ny    , 2*_nx+3*_ny -1 ,LRTB);
  mc.copyMatrixSegment(10, 2*_nx+4*_ny -1 , 4*_nx+4*_ny -1 ,TBRL);
  mc.copyMatrixSegment(11, 2*_nx+3*_ny -1 , 4*_nx+4*_ny -1 ,TBRL);
  mc.copyMatrixSegment(12, 4*_nx+4*_ny -1 , 4*_nx+2*_ny    ,RLBT);
  mc.copyMatrixSegment(13, 4*_nx+4*_ny -1 , 4*_nx+3*_ny    ,RLBT);
  mc.copyMatrixSegment(14, 4*_nx+4*_ny -1 , 4*_nx+3*_ny -1 ,TBRL);
  mc.copyMatrixSegment(15, 4*_nx+3*_ny -1 , 4*_nx+3*_ny -1 ,TBRL);

  //q2//
  mc.copyMatrixSegment(16, 2*_nx+4*_ny -1 , 2*_nx+2*_ny -1 ,TBRL);
  mc.copyMatrixSegment(17, 2*_nx+3*_ny -1 , 2*_nx+2*_ny -1 ,TBRL);
  mc.copyMatrixSegment(18, 4*_nx+4*_ny -1 , 2*_nx+0*_ny    ,RLBT);
  mc.copyMatrixSegment(19, 4*_nx+4*_ny -1 , 2*_nx+1*_ny    ,RLBT);
  mc.copyMatrixSegment(20, 4*_nx+2*_ny    , 0*_nx+0*_ny    ,BTLR);
  mc.copyMatrixSegment(21, 4*_nx+3*_ny    , 0*_nx+0*_ny    ,BTLR);
  mc.copyMatrixSegment(22, 4*_nx+2*_ny -1 , 0*_nx+0*_ny    ,RLBT);
  mc.copyMatrixSegment(23, 4*_nx+2*_ny -1 , 0*_nx+1*_ny    ,RLBT);

  //q3//
  mc.copyMatrixSegment(24, 2*_nx+2*_ny -1 , 2*_nx+0*_ny    ,RLBT);
  mc.copyMatrixSegment(25, 2*_nx+2*_ny -1 , 2*_nx+1*_ny    ,RLBT);
  mc.copyMatrixSegment(26, 2*_nx+0*_ny    , 0*_nx+0*_ny    ,BTLR);
  mc.copyMatrixSegment(27, 2*_nx+1*_ny    , 0*_nx+0*_ny    ,BTLR);
  mc.copyMatrixSegment(28, 0*_nx+0*_ny    , 0*_nx+2*_ny -1 ,LRTB);
  mc.copyMatrixSegment(29, 0*_nx+0*_ny    , 0*_nx+1*_ny -1 ,LRTB);
  mc.copyMatrixSegment(30, 0*_nx+0*_ny    , 0*_nx+2*_ny    ,BTLR);
  mc.copyMatrixSegment(31, 1*_nx+0*_ny    , 0*_nx+2*_ny    ,BTLR);

  //q0e0//
//  copyMatrix(src,dest,2*_nx,_ny,0, 0*_ny, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,0,1,1,0);
//  //q0e1//
//  copyMatrix(src,dest,2*_nx,_ny,0, 1*_ny, 2*_nx+1*_ny    , 2*_nx+2*_ny    ,0,1,1,0);
//  //q0e2//
//  copyMatrix(src,dest,2*_nx,_ny,0, 2*_ny, 0*_nx+0*_ny    , 2*_nx+4*_ny -1 ,1,0,0,-1);
//  //q0e3//
//  copyMatrix(src,dest,2*_nx,_ny,0, 3*_ny, 0*_nx+0*_ny    , 2*_nx+3*_ny -1 ,1,0,0,-1);
//  //q0e4//
//  copyMatrix(src,dest,2*_nx,_ny,0, 4*_ny, 0*_nx+2*_ny -1 , 4*_nx+4*_ny -1 ,0,-1,-1,0);
//  //q0e5//
//  copyMatrix(src,dest,2*_nx,_ny,0, 5*_ny, 0*_nx+1*_ny -1 , 4*_nx+4*_ny -1 ,0,-1,-1,0);
//  //q0e6//
//  copyMatrix(src,dest,2*_nx,_ny,0, 6*_ny, 0*_nx+2*_ny    , 4*_nx+4*_ny -1 ,1,0,0,-1);
//  //q0e7//
//  copyMatrix(src,dest,2*_nx,_ny,0, 7*_ny, 0*_nx+2*_ny    , 4*_nx+3*_ny -1 ,1,0,0,-1);

//  //q1e0//
//  copyMatrix(src,dest,2*_nx,_ny,0, 8*_ny, 2*_nx+2*_ny    , 2*_nx+4*_ny    ,0,1,1,0);

  _result->lock.unlock();
  hist.lock.unlock();
}


// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains postprocessors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <algorithm>
#include <cctype>

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
   * @param src reference to the linearized source matrix
   * @param dest reference to the linearized destination matrix
   * @param segment the index of the segment to be copied
   * @param destColStart dest column index where the src segment starts
   * @param destRowStart dest row index where the src segment starts
   * @param rot refernce to the rotor element that tells how the src segement is
   *            oriented in the dest matrix.
   */
  void operator()(const HistogramFloatBase::storage_t& src,
                  HistogramFloatBase::storage_t& dest,
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





// --------------convert cspad 2 cheetah--------------------

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






// --------------convert cspad 2 quasi laboratory --------------------


pp1601::pp1601(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key),
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
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = new Histogram2DFloat(2*(2*_nx+2*_ny),2*(2*_nx+2*_ny));

  _copyMatrixSegment =
      std::tr1::shared_ptr<SegmentCopier>(new SegmentCopier(2*_nx, _ny, 2*(2*_nx+2*_ny)));

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
  copySegment(src,dest, 0, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src,dest, 0, 2*_nx+0*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src,dest, 1, 2*_nx+1*_ny    , 2*_nx+2*_ny    ,_BTLR);
  copySegment(src,dest, 2, 0*_nx+0*_ny    , 2*_nx+4*_ny -1 ,_LRTB);
  copySegment(src,dest, 3, 0*_nx+0*_ny    , 2*_nx+3*_ny -1 ,_LRTB);
  copySegment(src,dest, 4, 0*_nx+2*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src,dest, 5, 0*_nx+1*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src,dest, 6, 0*_nx+2*_ny    , 4*_nx+4*_ny -1 ,_LRTB);
  copySegment(src,dest, 7, 0*_nx+2*_ny    , 4*_nx+3*_ny -1 ,_LRTB);

  //q1//
  copySegment(src,dest, 8, 2*_nx+2*_ny    , 2*_nx+4*_ny -1 ,_LRTB);
  copySegment(src,dest, 9, 2*_nx+2*_ny    , 2*_nx+3*_ny -1 ,_LRTB);
  copySegment(src,dest,10, 2*_nx+4*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src,dest,11, 2*_nx+3*_ny -1 , 4*_nx+4*_ny -1 ,_TBRL);
  copySegment(src,dest,12, 4*_nx+4*_ny -1 , 4*_nx+2*_ny    ,_RLBT);
  copySegment(src,dest,13, 4*_nx+4*_ny -1 , 4*_nx+3*_ny    ,_RLBT);
  copySegment(src,dest,14, 4*_nx+4*_ny -1 , 4*_nx+2*_ny -1 ,_TBRL);
  copySegment(src,dest,15, 4*_nx+3*_ny -1 , 4*_nx+2*_ny -1 ,_TBRL);

  //q2//
  copySegment(src,dest,16, 2*_nx+4*_ny -1 , 2*_nx+2*_ny -1 ,_TBRL);
  copySegment(src,dest,17, 2*_nx+3*_ny -1 , 2*_nx+2*_ny -1 ,_TBRL);
  copySegment(src,dest,18, 4*_nx+4*_ny -1 , 2*_nx+0*_ny    ,_RLBT);
  copySegment(src,dest,19, 4*_nx+4*_ny -1 , 2*_nx+1*_ny    ,_RLBT);
  copySegment(src,dest,20, 4*_nx+2*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src,dest,21, 4*_nx+3*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src,dest,22, 4*_nx+2*_ny -1 , 0*_nx+0*_ny    ,_RLBT);
  copySegment(src,dest,23, 4*_nx+2*_ny -1 , 0*_nx+1*_ny    ,_RLBT);

  //q3//
  copySegment(src,dest,24, 2*_nx+2*_ny -1 , 2*_nx+0*_ny    ,_RLBT);
  copySegment(src,dest,25, 2*_nx+2*_ny -1 , 2*_nx+1*_ny    ,_RLBT);
  copySegment(src,dest,26, 2*_nx+0*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src,dest,27, 2*_nx+1*_ny    , 0*_nx+0*_ny    ,_BTLR);
  copySegment(src,dest,28, 0*_nx+0*_ny    , 0*_nx+2*_ny -1 ,_LRTB);
  copySegment(src,dest,29, 0*_nx+0*_ny    , 0*_nx+1*_ny -1 ,_LRTB);
  copySegment(src,dest,30, 0*_nx+0*_ny    , 0*_nx+2*_ny    ,_BTLR);
  copySegment(src,dest,31, 0*_nx+1*_ny    , 0*_nx+2*_ny    ,_BTLR);


  _result->lock.unlock();
  hist.lock.unlock();
}



// --------------convert cspad 2 laboratory --------------------

pp1602::pp1602(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp1602::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _imagePP = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_imagePP && ret)) return;

//  _result = new Histogram2DFloat(_na*_nx,_na*_ny);

  createHistList(2*NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" +  _key + "' will convert Histogram in " +
           "PostProcessor '" +  _imagePP->key() + " into lab frame" +
           ". Condition is '" + _condition->key() + "'");
}

namespace cass
{
namespace geomparsing
{
struct asciiInfo_t
{
  long int min_fs;
  long int min_ss;
  long int max_fs;
  long int max_ss;
  string badrow_direction;
  double res;
  string clen;
  double corner_x;
  double corner_y;
  long int no_index;
  double x_fs;
  double x_ss;
  double y_fs;
  double y_ss;
};
}
}

void pp1602::generateLookupTable(const string &filename)
{
  map<string,geomparsing::asciiInfo_t> geomInfos;

  ifstream geomFile (filename.c_str());
  if (!geomFile.is_open())
    throw invalid_argument("blah: could not open file");

  string line;
  while(!geomFile.eof())
  {
    getline(geomFile, line);

    /** get asic string, value name and value (as string) from line */
    const string asic(line.substr(0,line.find('/')));
    const string valueNameAndValue(line.substr(line.find('/')));
    const string valueName(valueNameAndValue.substr(0,valueNameAndValue.find('=')));
    string valueString(valueNameAndValue.substr(valueNameAndValue.find('=')));

    /** depending on the value name retrieve the value as right type */
    char * pEnd;
    if (valueName == "min_fs")
      geomInfos[asic].min_fs = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "min_ss")
      geomInfos[asic].min_ss = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "max_fs")
      geomInfos[asic].max_fs = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "max_ss")
      geomInfos[asic].max_ss = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "badrow_direction")
    {
      valueString.erase(remove(valueString.begin(), valueString.end(), ' '), valueString.end());
      geomInfos[asic].badrow_direction = valueString;
    }
    else if (valueName == "res")
      geomInfos[asic].res = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "clen")
    {
      valueString.erase(remove(valueString.begin(), valueString.end(), ' '), valueString.end());
      geomInfos[asic].clen = valueString;
    }
    else if (valueName == "corner_x")
      geomInfos[asic].corner_x = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "corner_y")
      geomInfos[asic].corner_y = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "no_index")
      geomInfos[asic].no_index = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "fs")
    {
      double x(0),y(0);
      const string first(valueString.substr(0,valueString.find(' ')));
      if (first.find('x') != string::npos)
        x = std::strtod(first.c_str(),&pEnd);
      else if (first.find('y') != string::npos)
        y = std::strtod(first.c_str(),&pEnd);
      else
        throw runtime_error("pp1602: bad parsing");
      const string second(valueString.substr(valueString.find(' ')));
      if (second.find('x') != string::npos)
        x = std::strtod(second.c_str(),&pEnd);
      else if (second.find('y') != string::npos)
        y = std::strtod(second.c_str(),&pEnd);
      else
        throw runtime_error("pp1602: bad parsing");
      geomInfos[asic].x_fs = x;
      geomInfos[asic].y_fs = y;
    }
    else if (valueName == "ss")
    {
      geomInfos[asic].x_fs = std::strtol(valueString.c_str(),&pEnd,10);
      geomInfos[asic].y_fs = std::strtol(valueString.c_str(),&pEnd,10);
    }
    else
      throw runtime_error("pp1602::generateLookupTable: param does not exist");
  }
}

void pp1602::process(const CASSEvent &evt)
{
  /** Get the input histogram and its memory */
  const Histogram2DFloat &imageHist
      (dynamic_cast<const Histogram2DFloat&>((*_imagePP)(evt)));
  const HistogramFloatBase::storage_t& srcImage(imageHist.memory()) ;

  /** get result image and its memory */
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(*_result));
  HistogramFloatBase::storage_t& destImage(result.memory());

  /** lock resources */
  imageHist.lock.lockForRead();
  result.lock.lockForWrite();

  /** iterate through the src image and put its pixels at the location in the
   *  destination that is directed in the lookup table
   */
  HistogramFloatBase::storage_t::const_iterator pixel(srcImage.begin());
  HistogramFloatBase::storage_t::const_iterator imageEnd(srcImage.end());

  lookupTable_t::const_iterator convert(_lookupTable.begin());

  for (; pixel != imageEnd; ++pixel, ++convert)
    destImage[*convert] = *pixel;

  /** relfect that only 1 event was processed and release resources */
  result.nbrOfFills()=1;
  result.lock.unlock();
  imageHist.lock.unlock();
}


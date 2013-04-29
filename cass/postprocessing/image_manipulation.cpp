// Copyright (C) 2012 Lutz Foucar

/**
 * @file image_manipulation.h file contains postprocessors that will manipulate
 *                            2d histograms
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <tr1/functional>

#include <QtCore/QString>
#include "pdsdata/cspad/Detector.hh"

#include "image_manipulation.h"

#include "cass_settings.h"
#include "histogram.h"
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

namespace cass
{
namespace geometryInfo
{
struct asicInfo_t
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

struct pos_t
{
  typedef double x_t;
  typedef double y_t;
  x_t x;
  y_t y;
};

typedef vector<pos_t> conversion_t;

pos_t minus(const pos_t& minuent, const pos_t &subtrahend )
{
  pos_t pos(minuent);
  pos.x -= subtrahend.x;
  pos.y -= subtrahend.y;
  return pos;
}

size_t linearizeComponents(const pos_t &pos, const size_t nCols)
{
  const size_t col(static_cast<size_t>(pos.x + 0.5));
  const size_t row(static_cast<size_t>(pos.y + 0.5));
  return (row*nCols + col);
}

conversion_t generateConversionMap(const string &filename, const size_t sizeOfSrc, const size_t nSrcCols, const bool convertFromCheetahToCASS)
{
  typedef map<string,asicInfo_t> asicInfoMap_t;
  asicInfoMap_t geomInfos;
  conversion_t src2lab(sizeOfSrc);

  /** open file @throw invalid_argument when it could not be opened */
  ifstream geomFile (filename.c_str());
  if (!geomFile.is_open())
    throw invalid_argument(" pp1602::generateLookupTable(): could not open file '" +
                           filename +"'");

  /** read the file line by line */
  string line;
  while(!geomFile.eof())
  {
    getline(geomFile, line);

    /** if there is no '/' in line skip line */
    if (line.find('/') == string::npos)
      continue;

    /** get asic string, value name and value (as string) from line */
    const string asic(line.substr(0,line.find('/')));
    const string valueNameAndValue(line.substr(line.find('/')+1));
    string valueName(valueNameAndValue.substr(0,valueNameAndValue.find('=')));
    string valueString(valueNameAndValue.substr(valueNameAndValue.find('=')+1));

    /** eliminate whitespace from value name */
    valueName.erase(remove(valueName.begin(), valueName.end(), ' '), valueName.end());

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
      /** if value is fs then parse the string containing the 2 numbers */
      pEnd = &valueString[0];
      for (int i(0); i < 2 ; ++i)
      {
        const double number = strtod(pEnd,&pEnd);
        if (pEnd[0] == 'x')
          geomInfos[asic].x_fs = number;
        else if (pEnd[0] == 'y')
          geomInfos[asic].y_fs = number;
        else
          throw runtime_error(string("pp1602: Cannot assign '") + pEnd[0] + "' to x or y");
        ++pEnd;
      }
    }
    else if (valueName == "ss")
    {
      pEnd = &valueString[0];
      for (int i(0); i < 2 ; ++i)
      {
        const double number = strtod(pEnd,&pEnd);
        if (pEnd[0] == 'x')
          geomInfos[asic].x_ss = number;
        else if (pEnd[0] == 'y')
          geomInfos[asic].y_ss = number;
        else
          throw runtime_error(string("pp1602: Cannot assign '") + pEnd[0] + "' to x or y");
        ++pEnd;
      }
    }
  }

  /** go through all defined asics */
  asicInfoMap_t::iterator it(geomInfos.begin());
  for (; it != geomInfos.end(); ++it)
  {
    asicInfo_t& ai(it->second);

    /** if requested transform the start and end positions from the cheetah
     *  layout to the raw cass layout
     */
    if (convertFromCheetahToCASS)
    {
      const int nx(Pds::CsPad::MaxRowsPerASIC);
      const int ny(Pds::CsPad::ColumnsPerASIC);
      const int quad(ai.min_fs/(2*nx));
      const int asicRow(ai.min_ss/(1*ny));
      const int xbegin(ai.min_fs/(1*nx) % 2);
      const int ybegin(quad*2*4+asicRow);

      ai.min_fs = xbegin*nx;
      ai.max_fs = xbegin*nx + nx-1;

      ai.min_ss = ybegin*ny;
      ai.max_ss = ybegin*ny + ny-1;
    }


    /** go through all pixels of this asics module */
    const int rowAsicRange(ai.max_ss - ai.min_ss);
    const int colAsicRange(ai.max_fs - ai.min_fs);
    for (int rowInAsic = 0; rowInAsic <= rowAsicRange; ++rowInAsic)
    {
      for (int colInAsic = 0; colInAsic <= colAsicRange; ++colInAsic)
      {
        /** find the position in the lab frame (in pixel units) of the current
          *  position (colInAsic,rowInAsic) in the asic
          */
        double xInLab = ai.x_fs*colInAsic + ai.x_ss*rowInAsic + ai.corner_x;
        double yInLab = ai.y_fs*colInAsic + ai.y_ss*rowInAsic + ai.corner_y;

        /** determine where the current position in the asic is in the src image */
        int colInSrc = ai.min_fs+colInAsic;
        int rowInSrc = ai.min_ss+rowInAsic;

        /** find position in the linearized array */
        int idxInSrc = rowInSrc * nSrcCols + colInSrc;

        /** check if whats been given in the geomfile goes together with the src */
        if  (idxInSrc >= static_cast<int>(sizeOfSrc))
          throw out_of_range("generateConversionMap(): The generated index '" +
                             toString(idxInSrc) + "' is too big for the src with size '"+
                             toString(sizeOfSrc) +"'");

        /** remember what x,y position in the lab does this position in the
          *  asic correspond to
          */
        src2lab[idxInSrc].x = xInLab;
        src2lab[idxInSrc].y = yInLab;
      }
    }
  }
  return src2lab;
}
}//end namespace geometryInfo
}//end namespace cass

void pp1602::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _imagePP = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_imagePP && ret)) return;

  _filename = s.value("GeometryFilename","cspad.geom").toString().toStdString();
  _convertCheetahToCASSLayout = s.value("ConvertCheetahToCASSLayout",true).toBool();

  setup(dynamic_cast<const Histogram2DFloat&>(_imagePP->getHist(0)));

}

void pp1602::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  /** return when there is no incomming histogram */
  if(!in)
    return;
  /** return when the incomming histogram is not a direct dependant */
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;

  setup(dynamic_cast<const Histogram2DFloat&>(*in));

  /** notify all pp that depend on us that our histograms have changed */
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp1602::setup(const Histogram2DFloat &srcImageHist)
{
  _lookupTable.resize(srcImageHist.memory().size());
  try
  {
    using namespace geometryInfo;
    conversion_t src2lab = generateConversionMap(_filename,
                                                 srcImageHist.memory().size(),
                                                 srcImageHist.axis()[HistogramBackend::xAxis].size(),
                                                 _convertCheetahToCASSLayout);

    /** get the minimum position in lab x and y */
    pos_t min;
    min.x = min_element(src2lab.begin(),src2lab.end(),
                        bind(less<pos_t::x_t>(),
                             bind<pos_t::x_t>(&pos_t::x,_1),
                             bind<pos_t::x_t>(&pos_t::x,_2)))->x;
    min.y = min_element(src2lab.begin(),src2lab.end(),
                        bind(less<pos_t::y_t>(),
                             bind<pos_t::y_t>(&pos_t::y,_1),
                             bind<pos_t::y_t>(&pos_t::y,_2)))->y;

    /** move all values, such that they start at 0 and go to max
     *  \f$ pos.x -= min_x\f$
     *  \f$ pos.y -= min_y\f$
     */
    transform(src2lab.begin(),src2lab.end(),src2lab.begin(),bind(minus,_1,min));

    /** get the new maximum value of the shifted lab, which corresponds to the
     *  number of pixels that are required in the dest image, since all lab
     *  values are in pixel coordinates.
     */
    const double max_x = max_element(src2lab.begin(),src2lab.end(),
                                     bind(less<pos_t::x_t>(),
                                          bind<pos_t::x_t>(&pos_t::x,_1),
                                          bind<pos_t::x_t>(&pos_t::x,_2)))->x;
    const double max_y = max_element(src2lab.begin(),src2lab.end(),
                                     bind(less<pos_t::y_t>(),
                                          bind<pos_t::y_t>(&pos_t::y,_1),
                                          bind<pos_t::y_t>(&pos_t::y,_2)))->y;

    /** determine the dimensions of the destination image */
    const size_t nDestCols = static_cast<int>(max_x + 0.5)+1;
    const size_t nDestRows = static_cast<int>(max_y + 0.5)+1;

    /** convert the positions in the lab space (pixel units) to linearized indizes
     *  in the destination image
     *  \f$ _lookuptable = round(src2lab.x) + round(src2lab.y)*nDestCols \f$
     */
    transform(src2lab.begin(),src2lab.end(),_lookupTable.begin(),
              bind(linearizeComponents,_1,nDestCols));

    /** check if the boundaries are ok, @throw out of range if not. */
    if(nDestCols*nDestRows <= *max_element(_lookupTable.begin(),_lookupTable.end()))
      throw out_of_range("pp1602::setup: '" + _key + "' the maximum index in the lookup table '" +
                         toString(*max_element(_lookupTable.begin(),_lookupTable.end())) +
                         "' does not fit with the destination size of '" +
                         toString(nDestCols*nDestRows) + "'");

    /** create the destination image and setup the histlist */
    _result = new Histogram2DFloat(nDestCols , nDestRows);
    createHistList(2*NbrOfWorkers);

    Log::add(Log::INFO,"PostProcessor '" +  _key + "' will convert Histogram in " +
             "PostProcessor '" +  _imagePP->key() + " into lab frame" +
             ". Geometry Filename '" + _filename + "'"
             ". convert from cheetah to cass '" + (_convertCheetahToCASSLayout?"true":"false") + "'"
             ". Beam center is '" + toString(-min.x) + " x " + toString(-min.y) + "' pixels"+
             ". Condition is '" + _condition->key() + "'");
  }
  /** catch the out of range errors and intialize the lookup table and the
   *  image with bogus. Hopefully once everything resizes to the correct image
   *  size, no errors will be thrown anymore
   */
  catch(const out_of_range &error)
  {
    Log::add(Log::DEBUG0,"Postprocessor '" + _key +
             "': geomfile settings do not fit. Error is '" + error.what() +
             "'. Initializing histogram with bogus info.");
    fill(_lookupTable.begin(),_lookupTable.end(),0);
    _result = new Histogram2DFloat(1,1);
    createHistList(2*NbrOfWorkers);
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
  HistogramFloatBase::storage_t::const_iterator srcpixel(srcImage.begin());
  HistogramFloatBase::storage_t::const_iterator srcImageEnd(srcImage.end());

  lookupTable_t::const_iterator idx(_lookupTable.begin());

  for (; srcpixel != srcImageEnd; ++srcpixel, ++idx)
    destImage[*idx] = *srcpixel;

  /** relfect that only 1 event was processed and release resources */
  result.nbrOfFills()=1;
  result.lock.unlock();
  imageHist.lock.unlock();
}


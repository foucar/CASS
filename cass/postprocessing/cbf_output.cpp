//Copyright (C) 2012 Lutz Foucar

/**
 * @file cbf_output.cpp output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>

#include <stdint.h>
#include <iomanip>
#include <fstream>

#include "cbf_output.h"

#include "histogram.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.h"
#include "log.h"
#include "convenience_functions.h"
#include "cbf_handle.hpp"

using namespace cass;
using namespace std;

//namespace cass
//{
//namespace CBF
//{
///** write cbf file
// *
// * @param filename the filename to write the cbf to
// * @param data the data that should be written
// * @param nx number of columns of image
// * @param ny number of rows of image
// *
// * @author Stephan Kassemeyer
// */
//void write(const std::string &filename, const HistogramFloatBase::storage_t &data,
//           const int nx, const int ny)
//{
//  /** cbf parameters: */
//  int IOBUFSIZ = 4096;
//  char MARKBYTE[4] = {static_cast<char>(0x0C),
//                      static_cast<char>(0x01A),
//                      static_cast<char>(0x004),
//                      static_cast<char>(0x0D5)};
//
//  /** create the cbf file */
//  std::ofstream cbf_file;
//  cbf_file.open(filename.c_str(), std::ios::out|std::ios::binary);
//
//  /**  find out length of the compressed array: */
//  int nbytes = 0;
//  int pixvalue = 0;
//  int diff;
//  int absdiff;
//  for (int iadr=0; iadr<nx*ny; ++iadr)
//  {
//    diff = ((int) data[iadr]) - pixvalue;
//    pixvalue = (int) data[iadr];
//    absdiff = abs(diff);
//    ++nbytes;
//    if (absdiff < 128)
//      continue;
//    nbytes += 2;
//    if (absdiff < 32768)
//      continue;
//    nbytes += 4;
//  }
//
//  QString fn(QString::fromStdString(filename));
//  QString bn(QFileInfo(fn).baseName());
//  string fbn(bn.toStdString());
//
//  // write image header:
//  cbf_file << "###CBF: Version July 2012 generated by CASS" << "\r\n";
//  cbf_file << "" << "\r\n";
//  cbf_file << fbn << "\r\n";
//  cbf_file << "" << "\r\n";
//  cbf_file << "_array_data.header_convention \"XDS special\"" << "\r\n";
//  cbf_file << "_array_data.header_contents" << "\r\n";
//  cbf_file << ";" << "\r\n";
//  cbf_file << ";" << "\r\n";
//  cbf_file << "" << "\r\n";
//  cbf_file << "_array_data.data" << "\r\n";
//  cbf_file << ";" << "\r\n";
//  cbf_file << "--CIF-BINARY-FORMAT-SECTION--" << "\r\n";
//  cbf_file << "Content-Type: application/octet-stream;" << "\r\n";
//  cbf_file << "     conversions=\"x-CBF_BYTE_OFFSET\"" << "\r\n";
//  cbf_file << "Content-Transfer-Encoding: BINARY" << "\r\n";
//  cbf_file << "X-Binary-Size:" << nbytes << "\r\n";
//  cbf_file << "X-Binary-ID: 1" << "\r\n";
//  cbf_file << "X-Binary-Element-Type: \"signed 32-bit integer\"" << "\r\n";
//  cbf_file << "X-Binary-Element-Byte-Order: LITTLE_ENDIAN" << "\r\n";
//  cbf_file << "X-Binary-Number-of-Elements:" << nx*ny << "\r\n";
//  cbf_file << "X-Binary-Size-Fastest-Dimension:" << nx << "\r\n";
//  cbf_file << "X-Binary-Size-Second-Dimension:" << ny << "\r\n";
//  cbf_file << "" << "\r\n";
//  cbf_file << MARKBYTE[0] << MARKBYTE[1] << MARKBYTE[2] << MARKBYTE[3];
//
//
//  // determine endianness
//  int step, first2, last2, first4, last4;
//  union
//  {
//    uint32_t ii;
//    char cc[4];
//  }
//  bint = {0x01020304};
//  if (bint.cc[0] == 1)
//  {
//    // big endian
//    step = -1;
//    first2=1; last2=0;
//    first4=3; last4=0;
//  }
//  else
//  {
//    // little endian
//    step = 1;
//    first2=0; last2=1;
//    first4=0; last4=3;
//  }
//
//  /** write histogram data
//   * compress image using the byte offset method and save as octet stream
//   */
//  pixvalue = 0;
//
//  signed char onebyte[1];
//  signed char twobytes[2];
//  signed char fourbytes[4];
//  int shortint;
//
//  for (int iadr=0; iadr<nx*ny; ++iadr)
//  {
//    diff = ((int) data[iadr]) - pixvalue;
//    absdiff = abs(diff);
//    pixvalue = (int)data[iadr];
//
//    onebyte[0] = -128;
//    if (absdiff < 128)
//      onebyte[0] = diff;
//    cbf_file << onebyte[0];
//    if (absdiff < 128)
//      continue;
//
//    shortint = -32768;
//    if (absdiff < 32768)
//      shortint = diff;
//    *((char*)(&twobytes)+0) = *((char*)(&shortint)+0);
//    *((char*)(&twobytes)+1) = *((char*)(&shortint)+1);
//
//    for (int ii=first2; ii!=last2+step; ii+=step)
//      cbf_file << twobytes[ii];
//    if (absdiff < 32768)
//      continue;
//
//    *((char*)(&fourbytes)+0) = *((char*)(&diff)+0);
//    *((char*)(&fourbytes)+1) = *((char*)(&diff)+1);
//    *((char*)(&fourbytes)+2) = *((char*)(&diff)+2);
//    *((char*)(&fourbytes)+3) = *((char*)(&diff)+3);
//
//    for (int ii=first4; ii!=last4+step; ii+=step)
//      cbf_file << fourbytes[ii];
//  }
//
//  /** terminate image data part and pad last record of file with zeros:*/
//  cbf_file << "--CIF-BINARY-FORMAT-SECTION----";
//  cbf_file << ";";
//
//  char zerobyte;
//  zerobyte = 0;
//  for (int ii=0; ii<IOBUFSIZ; ++ii)
//    cbf_file << zerobyte;
//
//  cbf_file.close();
//
//}
//}//end namespace cbf
//}//end namespace cass


pp1500::pp1500(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp1500::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  bool allDepsAreThere(true);

  string hName(s.value("HistName","Unknown").toString().toStdString());
  if (hName != "Unknown")
  {
    _pHist = setupDependency("",hName);
    allDepsAreThere = _pHist && allDepsAreThere;
  }

  string summaryName(s.value("SummaryName","Unknown").toString().toStdString());
  if (summaryName != "Unknown")
  {
    _summaryHist = setupDependency("",summaryName);
    allDepsAreThere = _summaryHist && allDepsAreThere;
  }

  bool ret (setupCondition());
  if (!(ret && allDepsAreThere))
    return;

  if (_pHist)
  {
    const HistogramBackend &hist(_pHist->result());
    if (hist.dimension() != 2)
      throw invalid_argument("pp1500: The histogram '" + _pHist->name()
                             + "' is not a 2d histogram");
  }

  if (_summaryHist)
  {
    const HistogramBackend &sHist(_summaryHist->result());
    if (sHist.dimension() != 2)
      throw invalid_argument("pp1500: The summary histogram '" + _summaryHist->name()
                             + "'is not a 2d histogram");
  }

  /** when requested add the first subdir to the filename and make sure that the
   *  directory exists.
   */
  _basefilename = s.value("FileBaseName",QString::fromStdString(_basefilename)).toString().toStdString();
  _maxFilePerSubDir = s.value("MaximumNbrFilesPerDir",-1).toInt();
  _filecounter = 0;
  if(_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::intializeDir(_basefilename);

  _hide = true;
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' CBF Writer: " +
           "Basename '" + _basefilename + "'" +
           (_pHist? " Histname '" + _pHist->name() +"'" :"") +
           (_summaryHist? " Summary Histogram '" + _summaryHist->name() + "'":"") +
           ". Condition is '" + _condition->name() + "'");
}

const HistogramBackend& pp1500::result(const CASSEvent::id_t)
{
  throw logic_error("pp1500::result: '"+name()+"' should never be called");
}

void pp1500::processEvent(const CASSEvent &evt)
{
  /** return if there is no histogram to be written */
  if (!_pHist)
    return;

  /** return if the condition for this pp is false */
  if (!_condition->result(evt.id()).isTrue())
    return;

  QMutexLocker locker(&_lock);

  /** increment subdir in filename when they should be distributed and the
   *  counter exeeded the maximum amount of files per subdir
   */
  if (_maxFilePerSubDir == _filecounter)
  {
    _filecounter = 0;
    _basefilename = AlphaCounter::increaseDirCounter(_basefilename);
  }
  ++_filecounter;

  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>(_pHist->result(evt.id())));
  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  QReadLocker lock(&hist.lock);

  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".cbf");
  CBF::write(filename,histdata,hist.shape());
}

void pp1500::aboutToQuit()
{
  /** return if there is no summary to be written */
  if (!_summaryHist)
    return;

  QMutexLocker locker(&_lock);
  const Histogram2DFloat& sHist
      (dynamic_cast<const Histogram2DFloat&>(_summaryHist->result()));
  const Histogram2DFloat::storage_t& data( sHist.memory() );

  QReadLocker lock(&sHist.lock);

  /** create filename from base filename, but first remove subdir from filename
   *   when they should be distributed
   */
  if (_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::removeAlphaSubdir(_basefilename);
  string filename(_basefilename + "_Summary.cbf");
  CBF::write(filename,data,sHist.shape());
}

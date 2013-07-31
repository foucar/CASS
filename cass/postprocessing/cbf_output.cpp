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

using namespace cass;
using namespace std;

namespace cass
{
namespace CBF
{
/** write cbf file
 *
 * @param filename the filename to write the cbf to
 * @param data the data that should be written
 * @param nx number of columns of image
 * @param ny number of rows of image
 *
 * @author Stephan Kassemeyer
 */
void write(const std::string &filename, const HistogramFloatBase::storage_t &data,
           const int nx, const int ny)
{
  /** cbf parameters: */
  int IOBUFSIZ = 4096;
  char MARKBYTE[4] = {0x0C,0x01A,0x004,0x0D5};

  /** create the cbf file */
  std::ofstream cbf_file;
  cbf_file.open(filename.c_str(), std::ios::out|std::ios::binary);

  /**  find out length of the compressed array: */
  int nbytes = 0;
  int pixvalue = 0;
  int diff;
  int absdiff;
  for (int iadr=0; iadr<nx*ny; ++iadr)
  {
    diff = ((int) data[iadr]) - pixvalue;
    pixvalue = (int) data[iadr];
    absdiff = abs(diff);
    ++nbytes;
    if (absdiff < 128)
      continue;
    nbytes += 2;
    if (absdiff < 32768)
      continue;
    nbytes += 4;
  }


  // write image header:
  cbf_file << "###CBF: Version July 2012 generated by CASS" << "\r\n";
  cbf_file << "" << "\r\n";
  cbf_file << "data_" << filename << "\r\n";
  cbf_file << "" << "\r\n";
  cbf_file << "_array_data.header_convention \"XDS special\"" << "\r\n";
  cbf_file << "_array_data.header_contents" << "\r\n";
  cbf_file << ";" << "\r\n";
  cbf_file << ";" << "\r\n";
  cbf_file << "" << "\r\n";
  cbf_file << "_array_data.data" << "\r\n";
  cbf_file << ";" << "\r\n";
  cbf_file << "--CIF-BINARY-FORMAT-SECTION--" << "\r\n";
  cbf_file << "Content-Type: application/octet-stream;" << "\r\n";
  cbf_file << "     conversions=\"x-CBF_BYTE_OFFSET\"" << "\r\n";
  cbf_file << "Content-Transfer-Encoding: BINARY" << "\r\n";
  cbf_file << "X-Binary-Size:" << nbytes << "\r\n";
  cbf_file << "X-Binary-ID: 1" << "\r\n";
  cbf_file << "X-Binary-Element-Type: \"signed 32-bit integer\"" << "\r\n";
  cbf_file << "X-Binary-Element-Byte-Order: LITTLE_ENDIAN" << "\r\n";
  cbf_file << "X-Binary-Number-of-Elements:" << nx*ny << "\r\n";
  cbf_file << "X-Binary-Size-Fastest-Dimension:" << nx << "\r\n";
  cbf_file << "X-Binary-Size-Second-Dimension:" << ny << "\r\n";
  cbf_file << "" << "\r\n";
//  cbf_file << 0x0C << 0x01A << 0x004 << 0x0D5;  // MARKBYTE
  cbf_file << MARKBYTE[0] << MARKBYTE[1] << MARKBYTE[2] << MARKBYTE[3];


  // determine endianness
  int step, first2, last2, first4, last4;
  union
  {
    uint32_t ii;
    char cc[4];
  } bint = {0x01020304};
  if (bint.cc[0] == 1)
  {
    // big endian
    step = -1;
    first2=1; last2=0;
    first4=3; last4=0;
  }
  else
  {
    // little endian
    step = 1;
    first2=0; last2=1;
    first4=0; last4=3;
  }

  /** write histogram data
   * compress image using the byte offset method and save as octet stream
   */
  pixvalue = 0;

  signed char onebyte[1];
  signed char twobytes[2];
  signed char fourbytes[4];
  int shortint;


  for (int iadr=0; iadr<nx*ny; ++iadr)
  {
    diff = ((int) data[iadr]) - pixvalue;
    absdiff = abs(diff);
    pixvalue = (int)data[iadr];

    onebyte[0] = -128;
    if (absdiff < 128)
      onebyte[0] = diff;
    cbf_file << onebyte[0];
    if (absdiff < 128)
      continue;

    shortint = -32768;
    if (absdiff < 32768)
      shortint = diff;
    *((char*)(&twobytes)+0) = *((char*)(&shortint)+0);
    *((char*)(&twobytes)+1) = *((char*)(&shortint)+1);
    //twobytes[0] = (shortint & 0xff00)>>8;
    //twobytes[1] = shortint & 0xff;

    for (int ii=first2; ii!=last2+step; ii+=step)
      cbf_file << twobytes[ii];
    if (absdiff < 32768)
      continue;

    *((char*)(&fourbytes)+0) = *((char*)(&diff)+0);
    *((char*)(&fourbytes)+1) = *((char*)(&diff)+1);
    *((char*)(&fourbytes)+2) = *((char*)(&diff)+2);
    *((char*)(&fourbytes)+3) = *((char*)(&diff)+3);

    //fourbytes[0] = (diff & 0xff000000 ) >> (8*3);
    //fourbytes[1] = (diff & 0x00ff0000 ) >> (8*2);
    //fourbytes[2] = (diff & 0x0000ff00 ) >> (8*1);
    //fourbytes[3] = (diff & 0x000000ff ) >> (8*0);

    for (int ii=first4; ii!=last4+step; ii+=step)
      cbf_file << fourbytes[ii];
  }

  /** terminate image data part and pad last record of file with zeros:*/
  cbf_file << "--CIF-BINARY-FORMAT-SECTION----";
  cbf_file << ";";

  char zerobyte;
  zerobyte = 0;
  for (int ii=0; ii<IOBUFSIZ; ++ii)
    cbf_file << zerobyte;

  cbf_file.close();

}
}//end namespace cbf
}//end namespace cass


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
  _pHist = setupDependency("HistName");
  _darkHist = setupDependency("DarkName");
  bool ret (setupCondition());
  if (!(ret && _pHist && _darkHist))
    return;
  const HistogramBackend &hist(_pHist->result());
  if (hist.dimension() != 2)
    throw invalid_argument("pp1500: The histogram that should be written to cbf is not a 2d histogram");
  const HistogramBackend &dark(_darkHist->result());
  if (dark.dimension() != 2)
    throw invalid_argument("pp1500: The histogram that contains the offset is not a 2d histogram");

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
           "' will write histograms '" + _pHist->name() +"' to cbf file with '" +
           _basefilename + "' as basename" +
           ". Darkname '" + _darkHist->name() + "'" +
           ". Condition is '" + _condition->name() + "'");
}

const HistogramBackend& pp1500::result(const CASSEvent::id_t)
{
  throw logic_error("pp1500::result: '"+name()+"' should never be called");
}

void pp1500::processEvent(const CASSEvent &evt)
{
  if (!_condition->result(evt.id()).isTrue())
    return;

  QMutexLocker locker(&_lock);

  /** increment subdir in filename when they should be distributed and the
   *  counter exeeded the maximum amount of files per subdir
   */
  if (_maxFilePerSubDir == _filecounter)
  {
    _filecounter = 0;
    _basefilename = AlphaCounter::increaseCounter(_basefilename);
  }
  ++_filecounter;

  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>(_pHist->result(evt.id())));
  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  QReadLocker lock(&hist.lock);

  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".cbf");
  CBF::write(filename,histdata,hist.shape().first,hist.shape().second);
}

void pp1500::aboutToQuit()
{
  QMutexLocker locker(&_lock);
  const Histogram2DFloat& dark
      (dynamic_cast<const Histogram2DFloat&>(_darkHist->result()));
  const Histogram2DFloat::storage_t& data( dark.memory() );

  QReadLocker lock(&dark.lock);

  /** create filename from base filename, but first remove subdir from filename
   *   when they should be distributed
   */
  if (_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::removeAlphaSubdir(_basefilename);
  string filename(_basefilename + "_Dark.cbf");
  CBF::write(filename,data,dark.shape().first,dark.shape().second);
}

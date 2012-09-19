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

using namespace cass;
using namespace std;


pp1500::pp1500(PostProcessors &pp, const PostProcessors::key_t &key, const string& outfilename)
  : PostprocessorBackend(pp,key),
    _basefilename(outfilename)
{
  loadSettings(0);
}

void pp1500::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &hist(_pHist->getHist(0));
  if (hist.dimension() != 2)
    throw invalid_argument("pp1500: The histogram that should be written to hdf5 is not a 2d histogram");

  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will write histograms '" + _pHist->key() +"' to cbf file with '" +
           _basefilename + "' as basename. Condition is '" + _condition->key() + "'");
}

void pp1500::process(const CASSEvent &evt)
{
  QMutexLocker locker(&_lock);
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".cbf");

  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  /** cbf parameters: */
  int IOBUFSIZ = 4096;
  char MARKBYTE[4] = {0x0C,0x01A,0x004,0x0D5};

  /** create the cbf file */
  std::ofstream cbf_file;
  cbf_file.open(filename.c_str(), std::ios::out|std::ios::binary);
  int nx = hist.shape().first;
  int ny = hist.shape().second;


  /**  find out length of the compressed array: */
  int nbytes = 0;
  int pixvalue = 0;
  int diff;
  int absdiff;
  for (int iadr=0; iadr<nx*ny; ++iadr)
  {
    diff = ((int) histdata[iadr]) - pixvalue;
    pixvalue = (int) histdata[iadr];

    absdiff = abs(diff);
    ++nbytes;
    if (absdiff < 128)
      continue;
    nbytes += 2;
    if (absdiff < 32768)
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
    diff = ((int) histdata[iadr]) - pixvalue;
    absdiff = abs(diff);
    pixvalue = (int)histdata[iadr];

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

  hist.lock.lockForRead();


  /** close file */
  hist.lock.unlock();
}

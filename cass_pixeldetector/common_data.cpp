//Copyright (C) 2011 Lutz Foucar

/**
 * @file common_data.cpp data contains commonly used for all AdvancedDetectors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <cmath>

#include <QtCore/QFile>
#include <QtCore/QDateTime>

#include "common_data.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"
#include "mapcreator_base.h"
#include "pixeldetector_mask.h"
#include "hlltypes.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{
/** will read the file containing the offset and noise map in the hll format
 *
 * The Hll darkcal file format starts with a 1024 bit long header. Then an array
 * of staDataType objects containing more info than just the offset (mean) and
 * the noise (sigma) of the pixels.
 *
 * After that data a list of chars containing the bad pixel map is recorded.
 * The length of the arrays is according to the size of the frame.
 *
 * The header first contains a string to identify that it is a proper darkcal
 * file "HE pixel statistics map". Then it is followed by the following the
 * height, the width and the overall length of the frame. See HllFileHeader for
 * details.
 *
 * since the arrays for the offset and noise map are in the HLL layout, we
 * need to copy the individual information from the staDataType structure to
 * temporary arrays, which we then can convert to the layout that CASS is using.
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 *
 * @author Lutz Foucar
 */
void readHLLOffsetFile(const string &filename, CommonData& data)
{
  ifstream hllfile(filename.c_str(),ios::in);
  if (!hllfile.is_open())
  {
    throw invalid_argument("readHLLOffsetFile(): Error opening file '" +
                           filename + "'");
  }
  hllDataTypes::DarkcalFileHeader header;
  hllfile >> header;
  if ((string("HE pixel statistics map") != header.identifystring) ||
      (header.columns * header.rows != header.length))
  {
    throw invalid_argument("readHLLOffsetFile(): Error file '" + filename +
                           "' has either wrong header identify string '" +
                           header.identifystring + "' or the header information"
                           " is inconsistent columns * rows != length '" +
                           toString(header.columns)+ "*" + toString(header.rows) +
                           "!=" + toString(header.length) +"'");
  }
  hllDataTypes::statistics pixelStatistics[header.length];
  const size_t pixelStatisticsLength(sizeof(hllDataTypes::statistics)*header.length);
  hllfile.read(reinterpret_cast<char*>(pixelStatistics),pixelStatisticsLength);
  vector<char> badpixmap(header.length);
  hllfile.read(&badpixmap[0],header.length);

  frame_t hlloffsets(header.length);
  frame_t::iterator hlloffset(hlloffsets.begin());
  frame_t hllnoises(header.length);
  frame_t::iterator hllnoise(hllnoises.begin());
  hllDataTypes::statistics *pixelstatistic(&pixelStatistics[0]);
  for( size_t i(0); i < header.length; ++i, ++hlloffset, ++pixelstatistic )
  {
    *hlloffset = pixelstatistic->offset;
    *hllnoise = pixelstatistic->sigma;
  }
  QWriteLocker lock(&data.lock);
  data.offsetMap.resize(header.length);
  hllDataTypes::HLL2CASS(hlloffsets,data.offsetMap,header.rows,header.rows,header.columns);
  data.noiseMap.resize(header.length);
  hllDataTypes::HLL2CASS(hllnoises,data.noiseMap,header.rows,header.rows,header.columns);
  data.mask.resize(header.length);
  hllDataTypes:: HLL2CASS(badpixmap,data.mask,header.rows,header.rows,header.columns);
}

/** save the maps to a hll type darkcal file
 *
 * see readHLLOffsetFile for details about the fileformat. There is no need to
 * lock the maps since they are still locked by the lock started in createMaps
 *
 * @param filename the name of the file to write the data to
 * @param data the container including the maps to write to file
 *
 * @author Lutz Foucar
 */
void saveHLLOffsetFile(const string &filename, CommonData& data)
{
  ofstream out(filename.c_str(),ios::out|ios::trunc);
  if (!out.is_open())
  {
    throw invalid_argument("saveHLLOffsetFile(): Error opening file '" +
                           filename + "'");
  }
  hllDataTypes::DarkcalFileHeader header =
  {
    "HE pixel statistics map",
    data.columns*2,
    data.rows*0.5,
    data.rows * data.columns,
    ""
  };

  out.write(reinterpret_cast<char*>(&header),sizeof(hllDataTypes::DarkcalFileHeader));
  frame_t hlloffsets(data.offsetMap);
  hllDataTypes::CASS2HLL(data.offsetMap,hlloffsets,header.rows,header.rows,data.columns);
  frame_t hllnoises(data.noiseMap);
  hllDataTypes::CASS2HLL(data.noiseMap,hllnoises,header.rows,header.rows,data.columns);
  frame_t::const_iterator hlloffset(hlloffsets.begin());
  frame_t::const_iterator hllnoise(hllnoises.begin());
  hllDataTypes::statistics pixelStatistics[header.length];
  hllDataTypes::statistics *pixelstatistic(&pixelStatistics[0]);
  for(; hlloffset != hlloffsets.end(); ++hlloffset, ++hllnoise, ++pixelstatistic )
  {
    pixelstatistic->offset = *hlloffset;
    pixelstatistic->sigma = *hllnoise;
  }
  const size_t pixelStatisticsLength(sizeof(hllDataTypes::statistics)*header.length);
  out.write(reinterpret_cast<char*>(pixelStatistics),pixelStatisticsLength);
  CommonData::mask_t hllmask(data.mask);
  hllDataTypes::CASS2HLL(data.mask,hllmask,header.rows,header.rows,data.columns);
  const size_t maskLength(sizeof(char)*header.length);
  out.write(reinterpret_cast<char*>(&hllmask[0]),maskLength);
}

/** will read the file containing the offset and noise map in the former CASS format
 *
 * open the file then determine its size from which one can extract the size of
 * the frame. Then read data into temporary containers for double and then copy
 * the data to the final float containers
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 *
 * @author Lutz Foucar
 */
void readCASSOffsetFile(const string &filename, CommonData& data)
{
  ifstream in(filename.c_str(), ios::binary);
  if (!in.is_open())
  {
    throw invalid_argument("readCASSOffsetFile(): Error opening file '" +
                           filename + "'");
  }
  in.seekg(0,std::ios::end);
  const size_t size = in.tellg() / 2 / sizeof(double);
  in.seekg(0,std::ios::beg);
  vector<double> offsets(size);
  in.read(reinterpret_cast<char*>(&offsets[0]), size*sizeof(double));
  vector<double> noises(size);
  in.read(reinterpret_cast<char*>(&noises[0]), size*sizeof(double));
  QWriteLocker lock(&data.lock);
  copy(offsets.begin(),offsets.end(),data.offsetMap.begin());
  copy(noises.begin(),noises.end(),data.noiseMap.begin());
}

/** will save the file containing the offset and noise map in the former CASS format
 *
 * write the offset and noise map into a file. The values will just be written
 * in a binary stream of doubles
 * the maps are still locked by the createMaps lock when writing.
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 *
 * @author Lutz Foucar
 */
void saveCASSOffsetFile(const string &filename, CommonData& data)
{
  ofstream out(filename.c_str(), ios::binary);
  if (!out.is_open())
  {
    throw invalid_argument("saveCASSOffsetFile(): Error opening file '" +
                           filename + "'");
  }
  vector<double> offsets(data.offsetMap.size());
  copy(data.offsetMap.begin(),data.offsetMap.end(),offsets.begin());
  out.write(reinterpret_cast<char*>(&offsets[0]), offsets.size()*sizeof(double));
  vector<double> noises(data.noiseMap.size());
  copy(data.noiseMap.begin(),data.noiseMap.end(),noises.begin());
  out.write(reinterpret_cast<char*>(&noises[0]), noises.size()*sizeof(double));
}

/** will read the file containing the gain and cte corretion factors in the HLL format
 *
 * read in HLL gain/CTE correction format
 *
 * the first four lines look like the following
 *
 * HE File containing gain and CTE values
 * VERSION 3
 * HE       #Column   Gain    CTE0
 * GC          0          1   0.999977
 * [...]
 *
 * the third line and following lines have different spacing, depending on
 * how xonline is writing these files.
 *
 * column is related to the way the ccd is read out.  The ccd
 * consists of four segments
 *
 * BC
 * AD
 *
 * that are placed ABCD for the HLL file format with the
 * following numbers
 *
 * 1023 <- 512    1535 <- 1024
 *
 *    0 -> 512    1536 -> 2048
 *
 * The segments are read out bottom up for AD and
 * top down for BC (actually shifted to the corresponding edge).
 *
 * The memory representation is continuously
 *
 *  ...
 * 1024 -> 2047
 *    0 -> 1023
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 *
 * @author Mirko Scholz
 * @author Lutz Foucar
 */
void readHLLGainFile(const string &filename, CommonData& data)
{
  ifstream in(filename.c_str(), ios::binary);
  if (!in.is_open())
  {
    //fehler
  }
  char line[80];
  in.getline(line, 80);
  if (line != string("HE File"))
  {
    //error
    throw runtime_error("Wrong file format: " + std::string(line));
  }
  in.getline(line, 80);
  if (strncmp(line, "VERSION 3", 9))
  {
    throw runtime_error("Wrong file format: " + std::string(line));
  }
  in.getline(line, 80);

  vector<float> gains;
  vector<float> ctes;

  char g='G', c='C';
  size_t col;
  float gain, cte;

  while(true)
  {
    in>>g>>c>>col>>gain>>cte;
    if ((g != 'G') || (c != 'C'))
      break;
    gains.push_back(gain);
    ctes.push_back(cte);
  }

  //build up gain + cte map in HLL format
  frame_t hllgaincteMap;
  const size_t rows(512);
  const size_t columns(gains.size());
  for (size_t row(0); row < rows; ++row)
  {
    for (size_t column(0); column < columns; ++column)
    {
      hllgaincteMap.push_back(gains[column] / pow(ctes[column], row+1));
    }
  }

  //convert HLL format to CASS format
  QWriteLocker lock(&data.lock);
  hllDataTypes::HLL2CASS(hllgaincteMap,data.gain_cteMap,512,512,columns);
}

/** will create the final correction map from the info stored in the other maps
 *
 * @param data the data storage that is used to create the maps from
 *
 * @author Lutz Foucar
 */
void createCorrectionMap(CommonData& data)
{
  QWriteLocker lock(&data.lock);
  frame_t::iterator corval(data.correctionMap.begin());
  frame_t::const_iterator noise(data.noiseMap.begin());
  CommonData::mask_t::const_iterator mask(data.mask.begin());
  for(;corval != data.correctionMap.end(); ++corval, ++noise, ++mask)
    *corval = *corval * *mask * (*noise < data.noiseThreshold);
}

} //end namespace pixeldetector
} //end namespace cass

//initialize static members//
CommonData::instancesmap_t CommonData::_instances;
QMutex CommonData::_mutex;

CommonData::shared_pointer CommonData::instance(const instancesmap_t::key_type& detector)
{
  QMutexLocker lock(&_mutex);
  if (_instances.find(detector) == _instances.end())
  {
    VERBOSEOUT(std::cout << "CommonData::instance(): creating an"
               <<" instance of the Pixel Detector Helper for detector '"<<detector
               <<"'"
               <<std::endl);
    _instances[detector] = CommonData::shared_pointer(new CommonData(detector));
  }
  return _instances[detector];
}

CommonData::CommonData(const instancesmap_t::key_type& /*detname*/)
{}

void CommonData::loadSettings(CASSSettings &s)
{
  s.beginGroup("CorrectionMaps");
  string mapcreatortype(s.value("MapCreatorType","none").toString().toStdString());
  _mapcreator = MapCreatorBase::instance(mapcreatortype);
  _mapcreator->loadSettings(s);
  string offsetfilename(s.value("InputOffsetNoiseFilename",
                                QString::fromStdString("darkcal_"+toString(detectorId)+".cal")).toString().toStdString());
  string offsetfiletype(s.value("InputOffsetNoiseFiletype","hll").toString().toStdString());
  if (offsetfiletype == "hll")
    readHLLOffsetFile(offsetfilename, *this);
  else if(offsetfiletype == "cass")
    readCASSOffsetFile(offsetfilename, *this);
  else
  {
    throw invalid_argument("CommonData::loadSettings: OffsetNoiseFiletype '" +
                           offsetfiletype + "' does not exist");
  }
  string gainfilename(s.value("CTEGainFilename","").toString().toStdString());
  string gainfiletype(s.value("CTEGainFiletype","hll").toString().toStdString());
  if (gainfiletype == "hll")
    readHLLGainFile(gainfilename, *this);
  else
  {
    throw invalid_argument("CommonData::loadSettings: CTEGainFiletype '" +
                           offsetfiletype + "' does not exist");
  }
  _outputOffsetFilename = (s.value("OutputOffsetNoiseFilename","darkcal").toString().toStdString());
  string outputoffsetfiletype(s.value("OutputOffsetNoiseFiletype","hll").toString().toStdString());
  if (outputoffsetfiletype == "hll")
    _saveTo = &saveHLLOffsetFile;
  else if(outputoffsetfiletype == "cass")
    _saveTo = &saveCASSOffsetFile;
  else
  {
    throw invalid_argument("CommonData::loadSettings: OutputOffsetNoiseFiletype '" +
                           outputoffsetfiletype + "' does not exist");
  }
  createCASSMask(*this,s);
  noiseThreshold = s.value("NoisyPixelThreshold",40000).toFloat();
  createCorrectionMap(*this);
  s.endGroup();
}

void CommonData::createMaps(const Frame &frame)
{
  QWriteLocker wlock(&lock);
  MapCreatorBase& createCorrectionMaps(*_mapcreator);
  createCorrectionMaps(frame);
}

void CommonData::saveMaps()
{
  string outname(_outputOffsetFilename + "_"+toString(detectorId) + "_" +
                 QDateTime::currentDateTime().toString("yyyyMMdd_HHmm").toStdString() +
                 ".cal");
  _saveTo(outname,*this);
  string linkname("darkcal_" + toString(detectorId) +".cal");
  if (!QFile::link(QString::fromStdString(outname),QString::fromStdString(linkname)))
    throw runtime_error("CommonData::saveMaps: could not create a link named '"+
                        linkname + "' that points to the outputfile '" + outname +"'");
}

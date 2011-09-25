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

#include "common_data.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"
#include "mapcreator_base.h"
#include "pixeldetector_mask.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{

/** convert a linearised matrix in the CASS format to the hll format
 *
 * the difference between the CASS and HLL is, that the CASS format has all 4
 * quadrants in a quadrat wheras in HLL they are aligned in a rectange. See
 * pixeldetector::readHLLGainFile for more detais.
 *
 * @tparam inputContainerType the type of the container containing the input
 * @tparam outputContainerType the type of the container containing the output
 * @param CASSMatrix the container containing the linearised input matrix
 * @param HLLMatrix the container containing the linearised out matrix
 * @param quadrantColumns the number of columns in one quadrant
 * @param quadrantRows the number of rows in one quadrant
 * @param CASSColumns the number of columns in the CASS input container
 *
 * @author Lutz Foucar
 */
template <typename inputContainerType, typename outputContainerType>
void CASS2HLL(const inputContainerType& CASSMatrix,
              outputContainerType& HLLMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t CASSColumns)
{
  typename inputContainerType::const_iterator cassquadrant0(CASSMatrix.begin());
  typename inputContainerType::const_iterator cassquadrant1(CASSMatrix.begin()+quadrantColumns);
  typename inputContainerType::const_reverse_iterator cassquadrant2(CASSMatrix.rbegin()+quadrantColumns);
  typename inputContainerType::const_reverse_iterator cassquadrant3(CASSMatrix.rbegin());

  typename outputContainerType::iterator HLL(HLLMatrix.begin());

  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(cassquadrant0,cassquadrant0+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant2,cassquadrant2+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant3,cassquadrant3+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant1,cassquadrant1+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);

    advance(cassquadrant0,CASSColumns);
    advance(cassquadrant1,CASSColumns);
    advance(cassquadrant2,CASSColumns);
    advance(cassquadrant3,CASSColumns);
  }
}

/** convert a linearised matrix in the hll format to the CASS format
 *
 * the difference between the CASS and HLL is, that the CASS format has all 4
 * quadrants in a quadrat wheras in HLL they are aligned in a rectange. See
 * pixeldetector::readHLLGainFile for more detais.
 *
 * @tparam inputContainerType the type of the container containing the input
 * @tparam outputContainerType the type of the container containing the output
 * @param HLLMatrix the container containing the linearised input matrix
 * @param CASSMatrix the container containing the linearised out matrix
 * @param quadrantColumns the number of columns in one quadrant
 * @param quadrantRows the number of rows in one quadrant
 * @param HLLColumns the number of columns in the HLL input container
 *
 * @author Lutz Foucar
 */
template <typename inputContainerType, typename outputContainerType>
void HLL2CASS(const inputContainerType& HLLMatrix,
              outputContainerType& CASSMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t HLLColumns)
{
  typename inputContainerType::const_iterator hllquadrant0(HLLMatrix.begin());
  typename inputContainerType::const_reverse_iterator hllquadrant1(HLLMatrix.rbegin()+2*quadrantColumns);
  typename inputContainerType::const_reverse_iterator hllquadrant2(HLLMatrix.rbegin()+1*quadrantColumns);
  typename inputContainerType::const_iterator hllquadrant3(HLLMatrix.begin()+3*quadrantColumns);

  typename outputContainerType::iterator cass(CASSMatrix.begin());

  //copy quadrant read to right side (lower in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant0,hllquadrant0+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant3,hllquadrant3+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant0,HLLColumns);
    advance(hllquadrant3,HLLColumns);
  }
  //copy quadrants read to left side (upper in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant1,hllquadrant1+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant2,hllquadrant2+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant1,HLLColumns);
    advance(hllquadrant2,HLLColumns);
  }
}

/** struct describing the statistics saved in a HLL Darkcal file
 *
 * copied from the pnCCD lib
 *
 * @author Peter Holl
 * @author Nils Kimmel
 */
struct staDataType
{
  /** internal use */
  double sum;

  /** offset mean value of pixel (raw) */
  double offset;

  /** noise sigma value of pixel */
  double sigma;

  /** internal use */
  double sumSq;

  /** internal use */
  int count;

  /** offset mean value of pixel (common mode corrected) */
  int16_t mean;
};

/** the file header structure of the hll darkcal file format
 *
 * derived from the code within the pnCCD lib
 *
 * @author Lutz Foucar
 */
struct HllFileHeader
{
  /** string to identify that it is a hll darkcal file
   *
   * should contain "HE pixel statistics map"
   */
  char identifystring[24];

  /** the width of the frames */
  uint32_t columns;

  /** the height of the frames */
  uint32_t rows;

  /** the overal length of the frame, if the matrix is linearized */
  uint32_t length;

  /** empty to fill the header up to 1024 bytes */
  char fillspace[988];
};

/** create an outputfilename from a filename prefix
 *
 * append the date and time to the file name as well as the detector id
 *
 * @return string containing filename with appended time and detector id
 * @param filename the filename that should be altered
 *
 * @author Lutz Foucar
 */
string createOutputFilename(const string& filename)
{
  string output;
#warning implement this
  return output;
}

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
  HllFileHeader header;
  hllfile.read(reinterpret_cast<char*>(&header),sizeof(HllFileHeader));
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
  staDataType pixelStatistics[header.length];
  const size_t pixelStatisticsLength(sizeof(staDataType)*header.length);
  hllfile.read(reinterpret_cast<char*>(pixelStatistics),pixelStatisticsLength);
  vector<char> badpixmap(header.length);
  hllfile.read(&badpixmap[0],header.length);

  frame_t hlloffsets(header.length);
  frame_t::iterator hlloffset(hlloffsets.begin());
  frame_t hllnoises(header.length);
  frame_t::iterator hllnoise(hllnoises.begin());
  staDataType *pixelstatistic(&pixelStatistics[0]);
  for( size_t i(0); i < header.length; ++i, ++hlloffset, ++pixelstatistic )
  {
    *hlloffset = pixelstatistic->offset;
    *hllnoise = pixelstatistic->sigma;
  }
  QWriteLocker lock(&data.lock);
  data.offsetMap.resize(header.length);
  HLL2CASS(hlloffsets,data.offsetMap,header.rows,header.rows,header.columns);
  data.noiseMap.resize(header.length);
  HLL2CASS(hllnoises,data.noiseMap,header.rows,header.rows,header.columns);
  data.mask.resize(header.length);
  HLL2CASS(badpixmap,data.mask,header.rows,header.rows,header.columns);
}

/** save the maps to a hll type darkcal file
 *
 * see readHLLOffsetFile for details about the fileformat
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
  HllFileHeader header =
  {
    "HE pixel statistics map",
    data.columns*2,
    data.rows*0.5,
    data.rows * data.columns
  };
  out.write(reinterpret_cast<char*>(&header),sizeof(HllFileHeader));
  frame_t hlloffsets(data.offsetMap);
  CASS2HLL(data.offsetMap,hlloffsets,header.rows,header.rows,data.columns);
  frame_t hllnoises(data.noiseMap);
  CASS2HLL(data.noiseMap,hllnoises,header.rows,header.rows,data.columns);
  frame_t::const_iterator hlloffset(hlloffsets.begin());
  frame_t::const_iterator hllnoise(hllnoises.begin());
  staDataType pixelStatistics[header.length];
  staDataType *pixelstatistic(&pixelStatistics[0]);
  for(; hlloffset != hlloffsets.end(); ++hlloffset, ++hllnoise, ++pixelstatistic )
  {
    pixelstatistic->offset = *hlloffset;
    pixelstatistic->sigma = *hllnoise;
  }
  const size_t pixelStatisticsLength(sizeof(staDataType)*header.length);
  out.write(reinterpret_cast<char*>(pixelStatistics),pixelStatisticsLength);
  CommonData::mask_t hllmask(data.mask);
  CASS2HLL(data.mask,hllmask,header.rows,header.rows,data.columns);
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
 * description
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
  QReadLocker lock(&data.lock);
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
  HLL2CASS(hllgaincteMap,data.gain_cteMap,512,512,columns);
}

/** will read the file containing the gain and cte corretion factors in the former CASS format
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 * @author Lutz Foucar
 */
void readCASSGainFile(const string &filename, CommonData& data)
{
  throw runtime_error("readCASSGainFile() has not been implemented yet");
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
  MapCreatorBase& createCorrectionMaps(*_mapcreator);
  createCorrectionMaps(frame);
}

void CommonData::saveMaps()
{
  string outname(createOutputFilename(_outputOffsetFilename));
  _saveTo(outname,*this);
#warning create link to output filename
}

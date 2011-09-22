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

#include "common_data.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"
#include "mapcreator_base.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{

/** convert a linearised matrix in the CASS format to the hll format
 *
 * @param
 *
 * @author Lutz Foucar
 */
void CASS2HLL(const frame_t& CASSMatrix,
              frame_t& HLLMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t CASSColumns)
{
  frame_t::const_iterator cassquadrant0(CASSMatrix.begin());
  frame_t::const_iterator cassquadrant1(CASSMatrix.begin()+quadrantColumns);
  frame_t::const_reverse_iterator cassquadrant2(CASSMatrix.rbegin()+quadrantColumns);
  frame_t::const_reverse_iterator cassquadrant3(CASSMatrix.rbegin());

  frame_t::iterator HLL(HLLMatrix.begin());

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
 * @param
 *
 * @author Lutz Foucar
 */
void HLL2CASS(const frame_t& HLLMatrix,
              frame_t& CASSMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t HLLColumns)
{
  frame_t::const_iterator hllquadrant0(HLLMatrix.begin());
  frame_t::const_reverse_iterator hllquadrant1(HLLMatrix.rbegin()+2*quadrantColumns);
  frame_t::const_reverse_iterator hllquadrant2(HLLMatrix.rbegin()+1*quadrantColumns);
  frame_t::const_iterator hllquadrant3(HLLMatrix.begin()+3*quadrantColumns);

  frame_t::iterator cass(CASSMatrix.begin());

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
 * @author Lutz Foucar
 */
void readHLLOffsetFile(const string &filename, CommonData& data)
{
  ifstream hllfile(filename.c_str(),std::ios::in);
  if (!hllfile.is_open())
  {
    //fehler
  }
  HllFileHeader header;
  hllfile.read(reinterpret_cast<char*>(&header),sizeof(HllFileHeader));
  if ((string("HE pixel statistics map") != header.identifystring) &&
      (header.columns * header.rows != header.length))
  {
    //fehler
  }
  staDataType pixelStatistics[header.length];
  const size_t pixelStatisticsLength(sizeof(staDataType)*header.length);
  hllfile.read(reinterpret_cast<char*>(pixelStatistics),pixelStatisticsLength);
  char badpixmap[header.length];
  hllfile.read(badpixmap,header.length);

  frame_t hlloffsets(header.length);
  frame_t::iterator hlloffset(hlloffsets.begin());
  staDataType *pixelStatistic(&pixelStatistics[0]);
  for( size_t i(0); i < header.length; ++i, ++hlloffset, ++pixelStatistic )
  {
    *hlloffset = pixelStatistic->offset;
  }
  frame_t hllnoises(header.length);
  frame_t::iterator hllnoise(hllnoises.begin());
  pixelStatistic = (&pixelStatistics[0]);
  for( size_t i(0); i < header.length; ++i, ++hllnoise, ++pixelStatistic )
  {
    *hllnoise = pixelStatistic->sigma;
  }
#warning "copy also the bad pix map contained in the darkcalfile"

  QWriteLocker lock(&data.lock);
  data.offsetMap.resize(header.length);
  HLL2CASS(hlloffsets,data.offsetMap,header.rows,header.rows,header.columns);
  data.noiseMap.resize(header.length);
  HLL2CASS(hllnoises,data.noiseMap,header.rows,header.rows,header.columns);
}

/** will read the file containing the offset and noise map in the former CASS format
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 * @author Lutz Foucar
 */
void readCASSOffsetFile(const string &filename, CommonData& data)
{
#warning "implement function"
}

/** will read the file containing the gain and cte corretion factors in the HLL format
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 * @author Lutz Foucar
 */
void readHLLGainFile(const string &filename, CommonData& data)
{
#warning "implement function"
}

/** will read the file containing the gain and cte corretion factors in the former CASS format
 *
 * @param filename the filename of file containing the offset and noise maps.
 * @param data the data storage where the info should be written to.
 * @author Lutz Foucar
 */
void readCASSGainFile(const string &filename, CommonData& data)
{
#warning "implement function"
//  in.read(reinterpret_cast<char*>(&(dp._offset[0])), dp._offset.size()*sizeof(double));
//  in.read(reinterpret_cast<char*>(&(dp._noise[0])), dp._noise.size()*sizeof(double));

}

/** will create the final correction map from the info stored in the other maps
 *
 * @param data the data storage that is used to create the maps from
 *
 * @author Lutz Foucar
 */
void createCorrectionMap(CommonData& data)
{
#warning "implement function"
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
  string offsetfilename(s.value("OffsetNoiseFilename","").toString().toStdString());
  string offsetfiletype(s.value("OffsetNoiseFiletype","hll").toString().toStdString());
  if (offsetfiletype == "hll")
    readHLLOffsetFile(offsetfilename, *this);
  else if(offsetfiletype == "cass")
    readCASSOffsetFile(offsetfilename, *this);
  else
  {
    stringstream ss;
    ss <<"CommonData::loadSettings: OffsetNoiseFiletype '"<<offsetfiletype
       <<"' does not exist";
    throw invalid_argument(ss.str());
  }
  string gainfilename(s.value("CTEGainFilename","").toString().toStdString());
  string gainfiletype(s.value("CTEGainFiletype","hll").toString().toStdString());
  if (gainfiletype == "hll")
    readHLLGainFile(gainfilename, *this);
  else if(gainfiletype == "cass")
    readCASSGainFile(gainfilename, *this);
  else
  {
    stringstream ss;
    ss <<"CommonData::loadSettings: CTEGainFiletype '"<<offsetfiletype
       <<"' does not exist";
    throw invalid_argument(ss.str());
  }
#warning "add mask"
  createCorrectionMap(*this);
  s.endGroup();
}

void CommonData::createMaps(const Frame &frame)
{
  MapCreatorBase& createCorrectionMaps(*_mapcreator);
  createCorrectionMaps(frame);
}

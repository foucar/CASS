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
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>

#include "common_data.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"
#include "mapcreator_base.h"
#include "pixeldetector_mask.h"
#include "hlltypes.h"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
//using Streaming::operator >>;

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
  using Streaming::operator >>;
  ifstream hllfile(filename.c_str(),ios::in);
  if (!hllfile.is_open())
  {
    Log::add(Log::WARNING,"readHllOffsetFile: Could not open '" + filename +
             "'. Skip loading the offset and noise map.");
    return; 
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
  vector<hllDataTypes::statistics> pixelStatistics(header.length);
  const size_t pixelStatisticsLength(sizeof(hllDataTypes::statistics)*header.length);
  hllfile.read(reinterpret_cast<char*>(&pixelStatistics.front()),pixelStatisticsLength);
  vector<char> badpixmap(header.length);
  hllfile.read(&badpixmap[0],header.length);

  frame_t hlloffsets(header.length);
  frame_t::iterator hlloffset(hlloffsets.begin());
  frame_t hllnoises(header.length);
  frame_t::iterator hllnoise(hllnoises.begin());
  vector<hllDataTypes::statistics>::iterator pixelstatistic(pixelStatistics.begin());
  for( size_t i(0); i < header.length; ++i, ++hlloffset, ++hllnoise, ++pixelstatistic )
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
  vector<hllDataTypes::statistics> pixelStatistics(header.length);
  vector<hllDataTypes::statistics>::iterator pixelstatistic(pixelStatistics.begin());
  for(; hlloffset != hlloffsets.end(); ++hlloffset, ++hllnoise, ++pixelstatistic )
  {
    pixelstatistic->offset = *hlloffset;
    pixelstatistic->sigma = *hllnoise;
  }
  const size_t pixelStatisticsLength(sizeof(hllDataTypes::statistics)*header.length);
  out.write(reinterpret_cast<char*>(&pixelStatistics.front()),pixelStatisticsLength);
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
    Log::add(Log::WARNING,"readCASSOffsetFile: Could not open '" + filename +
             "'. Skipping reading the noise and offset maps.");
    return;
  }
  in.seekg(0,std::ios::end);
  const size_t size = in.tellg() / 2 / sizeof(double);
  in.seekg(0,std::ios::beg);
  vector<double> offsets(size);
  in.read(reinterpret_cast<char*>(&offsets[0]), size*sizeof(double));
  vector<double> noises(size);
  in.read(reinterpret_cast<char*>(&noises[0]), size*sizeof(double));
  QWriteLocker lock(&data.lock);
  data.offsetMap.resize(size);
  copy(offsets.begin(),offsets.end(),data.offsetMap.begin());
  data.noiseMap.resize(size);
  copy(noises.begin(),noises.end(),data.noiseMap.begin());
}

/** will save the file containing the offset and noise map in the former CASS format
 *
 * write the offset and noise map into a file. The values will just be written
 * in a binary stream of doubles.
 *
 * The maps are still locked by the createMaps lock when writing.
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
 * read in HLL gain/CTE correction format from a given file. If no filename is given
 * just return from the function without doing anything.
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
  if (filename.empty())
    return;
  ifstream in(filename.c_str(), ios::binary);
  if (!in.is_open())
  {
    Log::add(Log::WARNING,"readHLLGainFile: File '" + filename +
             "' does not exist. Skipping reading the Gain/CTE file");
    return;
  }
  char line[80];
  in.getline(line, 80);
  if (line != string("HE File containing gain and CTE values"))
  {
    throw runtime_error("readHLLGainFile: Wrong file format: " + std::string(line));
  }
  in.getline(line, 80);
  if (strncmp(line, "VERSION 3", 9))
  {
    throw runtime_error("readHLLGainFile: Wrong file format: " + std::string(line));
  }
  in.getline(line, 80);

  vector<float> gains;
  vector<float> ctes;

  char g='G', c='C';
  int col;
  float gain, cte;

  while(true)
  {
    in>>g;
    in>>c;
    in>>skipws>>col;
    in>>skipws>>gain;
    in>>skipws>>cte;
//    cout << g << " " << c <<" "<<col<<" "<<gain<<" "<<cte<<endl;
    if ((g != 'G') || (c != 'C'))
      break;
    gains.push_back(gain);
    ctes.push_back(cte);
  }

  //build up gain + cte map in HLL format
  frame_t hllgaincteMap;
  const size_t rows(512);
  const size_t columns(gains.size());
//  cout << rows << " " << columns<<endl;
  for (size_t row(0); row < rows; ++row)
  {
    for (size_t column(0); column < columns; ++column)
    {
      hllgaincteMap.push_back(gains[column] / pow(ctes[column], static_cast<int>(row+1)));
    }
  }
  //convert HLL format to CASS format
  QWriteLocker lock(&data.lock);
  data.gain_cteMap.resize(hllgaincteMap.size());
  hllDataTypes::HLL2CASS(hllgaincteMap,data.gain_cteMap,512,512,columns);
}

/** will create the final correction map from the info stored in the other maps
 *
 * the correction value for a pixel is calculated using the following formular:
 *
 * \f[
 *  corval = ctegain \times corval \times maskval \times (
 *     0, & \text{if} noise < noisethreshold ;
 *     1, & \text{otherwise})
 * \f]
 *
 * @note one should lock this from the caller of this function
 *
 * @param data the data storage that is used to create the maps from
 *
 * @author Lutz Foucar
 */
void createCorrectionMap(CommonData& data)
{
  frame_t::iterator corval(data.correctionMap.begin());
  frame_t::const_iterator noise(data.noiseMap.begin());
  frame_t::const_iterator gain(data.gain_cteMap.begin());
  CommonData::mask_t::const_iterator mask(data.mask.begin());
  for(;corval != data.correctionMap.end(); ++corval, ++noise, ++mask, ++gain)
    *corval = *gain * *corval * *mask * (*noise < data.noiseThreshold) * (!qFuzzyCompare(*noise,0.f));
}

/** check whether the frame has the same size as the maps.
 *
 * if not resize the maps to the right size and initialize them with values, that
 * will let the hitfinder find no pixels and the correction make does not alter
 * the frame.
 *
 * this function relys on that the maps are locked. Usually they are since this
 * is called by the operators of the map creators. These are locked because
 * the function calling the operators will lock the maps before.
 *
 * @param frame the Frame to check
 * @param data the container for all maps.
 *
 * @author Lutz Foucar
 */
void isSameSize(const Frame& frame, CommonData& data)
{
  QWriteLocker(&data.lock);
  bool changed(false);
  if ((frame.columns * frame.rows) != static_cast<int>(data.offsetMap.size()))
  {
    Log::add(Log::WARNING,"isSameSize():The offsetMap does not have the right size '" +
             toString(data.offsetMap.size()) +
             "' to accommodate the frames with size '" +
             toString(frame.columns * frame.rows) +
             "'. Resizing the offsetMap");
    data.offsetMap.resize(frame.columns*frame.rows, 0);
    data.columns = frame.columns;
    data.rows = frame.rows;
    changed=true;
  }
  if ((frame.columns * frame.rows) != static_cast<int>(data.noiseMap.size()))
  {
    Log::add(Log::WARNING,"isSameSize():The noiseMap does not have the right size '" +
             toString(data.noiseMap.size()) +
             "' to accommodate the frames with size '" +
             toString(frame.columns * frame.rows) +
             "'. Resizing the noiseMap");
    data.noiseMap.resize(frame.columns*frame.rows, 4000);
    data.columns = frame.columns;
    data.rows = frame.rows;
    changed=true;
  }
  if ((frame.columns * frame.rows) != static_cast<int>(data.mask.size()))
  {
    Log::add(Log::WARNING,"isSameSize():The mask does not have the right size '" +
             toString(data.mask.size()) +
             "' to accommodate the frames with size '" +
             toString(frame.columns * frame.rows) +
             "'. Resizing the mask");
    data.mask.resize(frame.columns*frame.rows, 1);
    data.columns = frame.columns;
    data.rows = frame.rows;
    changed=true;
  }
  if ((frame.columns * frame.rows) != static_cast<int>(data.gain_cteMap.size()))
  {
    Log::add(Log::WARNING,"isSameSize():The gain_cteMap does not have the right size '" +
             toString(data.gain_cteMap.size()) +
             "' to accommodate the frames with size '" +
             toString(frame.columns * frame.rows) +
             "'. Resizing the gain_cteMap");
    data.gain_cteMap.resize(frame.columns*frame.rows, 1);
    data.columns = frame.columns;
    data.rows = frame.rows;
    changed=true;
  }
  if ((frame.columns * frame.rows) != static_cast<int>(data.correctionMap.size()))
  {
    Log::add(Log::WARNING,"isSameSize():The correctionMap does not have the right size '" +
             toString(data.correctionMap.size()) +
             "' to accommodate the frames with size '" +
             toString(frame.columns * frame.rows) +
             "'. Resizing the correctionMap");
    data.correctionMap.resize(frame.columns*frame.rows, 1);
    data.columns = frame.columns;
    data.rows = frame.rows;
    changed=true;
  }
  if(changed)
    data.createCorMap();
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
    Log::add(Log::DEBUG0,"CommonData::instance(): creating an instance of the" +
             string(" common data container for detector '") + detector + "'");
    _instances[detector] = CommonData::shared_pointer(new CommonData(detector));
  }
  return _instances[detector];
}

void CommonData::controlCalibration(const string& command)
{
  QMutexLocker locker(&_mutex);
  instancesmap_t::iterator instance(_instances.begin());
  instancesmap_t::const_iterator end(_instances.end());
  for (; instance != end; ++instance)
  {
    Log::add(Log::DEBUG1,"CommonData::controlCalibration: Starting calibration of '" +
             instance->first +"'");
    instance->second->_mapcreator->controlCalibration(command);
  }
}

CommonData::CommonData(const instancesmap_t::key_type& /*detname*/)
  : columns(1024),
    rows(1024),
    noiseThreshold(0),
    detectorId(-1),
    _settingsLoaded(false)
{}

void CommonData::loadSettings(CASSSettings &s)
{
  if (!_settingsLoaded)
  {
    string detectorname(s.group().split("/").back().toStdString());
    s.beginGroup("CorrectionMaps");
    string mapcreatortype(s.value("MapCreatorType","none").toString().toStdString());
    _mapcreator = MapCreatorBase::instance(mapcreatortype);
    _mapcreator->loadSettings(s);
    string offsetfilename(s.value("InputOffsetNoiseFilename",
                                  QString::fromStdString("darkcal_"+toString(detectorId)+".lnk")).toString().toStdString());
    string offsetfiletype(s.value("InputOffsetNoiseFiletype","hll").toString().toStdString());
    /** if filename is a link, try to deduce the real filename */
    QFileInfo offsetfilenameInfo(QString::fromStdString(offsetfilename));
    if (offsetfilenameInfo.isSymLink())
    {
      if (offsetfilenameInfo.exists())
        offsetfilename = offsetfilenameInfo.symLinkTarget().toStdString();
      else
        Log::add(Log::WARNING,"CommonData::loadSettings: The given offset filename '" +
                 offsetfilename + "' is a link that referes to a non existing file!");
    }
    if (offsetfilename != _inputOffsetFilename)
    {
      Log::add(Log::VERBOSEINFO, "CommonData::loadSettings(): Load Darkcal data " +
               string(" for detector with name '") + detectorname + "' which has id '" +
               toString(detectorId) + "' from file '" + offsetfilename +"'");
      _inputOffsetFilename = offsetfilename;
      if (offsetfiletype == "hll")
        readHLLOffsetFile(offsetfilename, *this);
      else if(offsetfiletype == "cass")
        readCASSOffsetFile(offsetfilename, *this);
      else
      {
        throw invalid_argument("CommonData::loadSettings: OffsetNoiseFiletype '" +
                               offsetfiletype + "' does not exist");
      }
    }
    string ctegainFiletype(s.value("CTEGainFiletype","hll").toString().toStdString());
    if (ctegainFiletype == "hll")
      _readGain = &readHLLGainFile;
    else
    {
      throw invalid_argument("CommonData::loadSettings: CTEGainFiletype '" +
                             offsetfiletype + "' does not exist");
    }
    _ctegainFilename = s.value("CTEGainFilename","").toString().toStdString();
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
    /** read the mask values */
    createCASSMask(*this,s);
    noiseThreshold = s.value("NoisyPixelThreshold",40000).toFloat();
    createCorMap();
    s.endGroup();
  }
  _settingsLoaded = true;
}

void CommonData::createMaps(const Frame &frame)
{
  if (_settingsLoaded)
  {
    _settingsLoaded = false;
    isSameSize(frame,*this);
  }
  MapCreatorBase& createOffsetNoiseMaps(*_mapcreator);
  createOffsetNoiseMaps(frame);
}

void CommonData::saveMaps()
{
  string outname(_outputOffsetFilename + "_"+toString(detectorId) + "_" +
                 QDateTime::currentDateTime().toString("yyyyMMdd_HHmm").toStdString() +
                 ".cal");
  _saveTo(outname,*this);
  string linkname("darkcal_" + toString(detectorId) +".lnk");
  if (QFile::exists(QString::fromStdString(linkname)))
    if(!QFile::remove(QString::fromStdString(linkname)))
      throw runtime_error("CommonData::saveMaps: could not remove already existing link '" +
                          linkname +"'");
  if (!QFile::link(QString::fromStdString(outname),QString::fromStdString(linkname)))
    throw runtime_error("CommonData::saveMaps: could not create a link named '"+
                        linkname + "' that points to the outputfile '" + outname +"'");
}

void CommonData::createCorMap()
{
  /** reset the correction map before reading the correction values */
  fill(correctionMap.begin(),correctionMap.end(),1.);
  _readGain(_ctegainFilename,*this);
  createCorrectionMap(*this);
}

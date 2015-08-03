// Copyright (C) 2014, 2015 Lutz Foucar

/**
 * @file sacla_converter.cpp contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QRegExp>

#include "sacla_converter.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.hpp"
#include "cass.h"
#include "log.h"

using namespace cass;
using namespace MachineData;
using namespace std;

/** retrieve the tile data from a detector
 *
 * retrieve the tile data of a detector and either normalize or copy it directly
 * to the right position within the frame
 *
 * @tparam T the type of the detector data
 *
 * @param tileParams parameters of the tile of the frame
 * @param runNbr the run number of the experiment
 * @param blNbr the Beamline number of the experiment
 * @param highTagNbr the high tag number for the experiment
 * @param tagNbr the tag number that is associated with the requested data
 *
 * @author Lutz Foucar
 */
template <typename T>
void retrieveTileData(SACLAConverter::detTileParams &tileParams,
                      const int runNbr, const int blNbr,
                      const int highTagNbr, const int tagNbr)
{
  /** determine the size of the data */
  const size_t size(tileParams.xsize * tileParams.ysize);

  /** prepare the buffer where the data should be loaded to and
   * retrieve the detector data
   */
  vector<T> buffer(size);
  int funcstatus(0);
  funcstatus = ReadDetData(&buffer.front(),tileParams.name.c_str(),
                           blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve data of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    tileParams.bytes_retrieved = 0.;
  }

  /** if tile should be normalized, use transform to copy the data, otherwise
   *  just copy the tile data to the frame
   */
  if (tileParams.normalize)
    transform(buffer.begin(), buffer.end(), tileParams.start,
              bind1st(multiplies<float>(),tileParams.relativeGain));
  else
    copy(buffer.begin(), buffer.end(), tileParams.start);

  /** set the datasize of the retrieved data */
  tileParams.bytes_retrieved = size * sizeof(uint16_t);
}


/** cache the non-changing parameters of a tile
 *
 * retrieve the non-changing parameters of the tiles and store them in the
 * tile parameters
 *
 * @return true in case all parameters were loaded correctly, false otherwise
 * @param tileParams the tile whos parameters should be cached.
 * @param runNbr the tile whos parameters should be cached.
 * @param blNbr the tile whos parameters should be cached.
 * @param highTagNbr the tile whos parameters should be cached.
 * @param tagNbr the tile whos parameters should be cached.
 *
 * @author Lutz Foucar
 */
bool cacheTileParams(SACLAConverter::detTileParams &tileParams, int runNbr,
                    int blNbr, int highTagNbr, int tagNbr)
{
  int funcstatus(0);
  /** the number of columns */
  funcstatus = ReadXSizeOfDetData(tileParams.xsize,tileParams.name.c_str(),
                                  blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: width of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  else
    Log::add(Log::INFO,"cacheDetParams: Tile '" + tileParams.name +
             "' has xsize '" + toString(tileParams.xsize) + "'");

  /** the number of rows */
  funcstatus = ReadYSizeOfDetData(tileParams.ysize,tileParams.name.c_str(),
                                  blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: height of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  else
    Log::add(Log::INFO,"cacheDetParams: Tile '" + tileParams.name +
             "' has ysize '" + toString(tileParams.ysize) + "'");

  /** the size of the data of the tile in bytes */
  funcstatus = ReadSizeOfDetData(tileParams.datasize_bytes,tileParams.name.c_str(),
                                 blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: datasize of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  else
    Log::add(Log::INFO,"cacheDetParams: Tile '" + tileParams.name +
             "' has datasize '" + toString(tileParams.datasize_bytes) + "' bytes");

  /** the size of the pixels of the tile */
  funcstatus = ReadPixelSize(tileParams.pixsize_um,tileParams.name.c_str(),
                             blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParamss: pixelsize of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  else
    Log::add(Log::INFO,"cacheDetParams: Tile '" + tileParams.name +
             "' has pixelsize '" + toString(tileParams.pixsize_um) + "' um");

  /** the data type of the tile */
  funcstatus = ReadDetDataType(tileParams.type,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: datatype of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  else
    Log::add(Log::INFO,"cacheDetParams: Tile '" + tileParams.name +
             "' has data type '" + toString(tileParams.type) + "'");

  return true;
}

SACLAConverter::SACLAConverter()
{}

void SACLAConverter::loadSettings()
{
  CASSSettings s;
  s.beginGroup("SACLAConverter");

  /** set the flag to retrieve the accelerator data */
  _retrieveAcceleratorData = s.value("RetrieveAcceleratorData",true).toBool();
  Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Will ") +
           (_retrieveAcceleratorData?"":"not ") + "retrieve the accelerator data");

  /** set the requested octal detectors */
  int size = s.beginReadArray("OctalPixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string detID(s.value("DetectorIDName","Invalid").toString().toStdString());
    /** skip if the detector name has not been set */
    if (detID == "Invalid")
      continue;
    _octalDetectors.push_back(pixDets_t::value_type());
    _octalDetectors.back().CASSID = s.value("CASSID",0).toInt();
    _octalDetectors.back().normalize = s.value("NormalizeToAbsGain",true).toBool();
    _octalDetectors.back().notLoaded = true;
    _octalDetectors.back().tiles.resize(8);
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add octal detector with CASSID '") +
             toString(_octalDetectors.back().CASSID) + "'");
    for (size_t i(0); i<_octalDetectors.back().tiles.size(); ++i)
      _octalDetectors.back().tiles[i].name = (detID + "-" + toString(i+1));
  }
  s.endArray();

  /** set the requested pixel detectors */
  size = s.beginReadArray("PixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string detID(s.value("DetectorIDName","Invalid").toString().toStdString());
    /** skip if the detector name has not been set */
    if (detID == "Invalid")
      continue;
    _pixelDetectors.push_back(pixDets_t::value_type());
    _pixelDetectors.back().CASSID = s.value("CASSID",0).toInt();
    _pixelDetectors.back().normalize = false;
    _pixelDetectors.back().notLoaded = true;
    _pixelDetectors.back().tiles.resize(1);
    _pixelDetectors.back().tiles[0].name = detID;
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add detector with CASSID '") +
             toString(_pixelDetectors.back().CASSID) + "'");
  }
  s.endArray();

  /** set the requested octal detectors */
  size = s.beginReadArray("DatabaseValues");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string machineValName(s.value("ValueName","Invalid").toString().toStdString());
    /** skip if the value name has not been set */
    if (machineValName == "Invalid")
      continue;
    /** retrieve the name that the value should have in the CASSEvent
     *  (in case non is given, default it to the machine value name
     */
    _machineVals.push_back(machineVals_t::value_type());
    _machineVals.back().databaseName = machineValName;
    _machineVals.back().cassName = s.value("CASSName",QString::fromStdString(machineValName)).toString().toStdString();
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add database value '") +
             _machineVals.back().databaseName + "' with CASSName '" +
             _machineVals.back().cassName + "'");
  }
  s.endArray();

  s.endGroup();
}

void SACLAConverter::cacheParameters(vector<int>::const_iterator first,
                                     vector<int>::const_iterator last,
                                     int blNbr, int runNbr, int highTagNbr)
{
  /** create the tag list from the iterators */
  vector<int> tagList(first,last);

  int funcstatus(0);
  /** for all requested beamline parameters retrieve the values for all tags in
   *  one go
   */
  machineVals_t::iterator machineValsIter(_machineVals.begin());
  machineVals_t::const_iterator machineValsEnd(_machineVals.end());
  for (; machineValsIter != machineValsEnd; ++machineValsIter)
  {
    MachineValue & mv(*machineValsIter);
    vector<string> machineValueStringList;
    funcstatus = ReadSyncDataList(&machineValueStringList,
                                  const_cast<char*>(mv.databaseName.c_str()),
                                  highTagNbr,tagList);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"SACLAConverter::cacheParameters could not cache values of '" +
               mv.databaseName + "' ErrorCode is '" + toString(funcstatus) + "'");
      continue;
    }
    /** check if as many parameters as tags given have been returned. In case
     *  this number is different, something bad happened
     */
    if (machineValueStringList.size() != tagList.size())
    {
      Log::add(Log::ERROR,"SACLAConverter:cacheParameters caching '" +
               mv.databaseName + "' did not return the right size");
      continue;
    }
    /** convert the retrieved database values into double numbers
     *  and put them into the cache
     */
    vector<int>::const_iterator tag(tagList.begin());
    vector<int>::const_iterator tagListEnd(tagList.end());
    vector<string>::const_iterator machine(machineValueStringList.begin());
    for (; tag != tagListEnd; ++tag, ++machine)
    {
      /** check if retrieved value can be converted to double, and if so add it
       *  to the machine data, otherwise issue an error and add a 0 into the
       *  cache for that given tag.
       *  @note the retrieved values might contain the unit of the value in the
       *        string, therefore one has to remove all characters from the string
       */
      QString machineValueQString(QString::fromStdString(*machine));
      machineValueQString.remove(QRegExp("V|C|pulse|a\\.u\\."));
      bool isDouble(false);
      double machineValue(machineValueQString.toDouble(&isDouble));
      if (isDouble)
        mv.values[*tag] = machineValue;
      else
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameters '" +
                 mv.databaseName + "' for tag '" + toString(*tag) +
                 "': String '" + *machine + "' which is altered to '" +
                 machineValueQString.toStdString() +
                 "' to remove units, cannot be converted to double. Setting it to 0");
        mv.values[*tag] = 0.;
      }
    }
  }

  /** for all pixel dets, which consist of only 1 tile, retrieve the
   *  non-changing parameters from the first image
   */
  pixDets_t::iterator pixelDetsIter(_pixelDetectors.begin());
  pixDets_t::const_iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
    if (cacheTileParams(pixelDetsIter->tiles[0],runNbr,blNbr,highTagNbr,*first))
      pixelDetsIter->notLoaded = false;

  /** for all octal dets retrieve the non changing parameters from the first
   *  image
   */
  pixDets_t::iterator octalDetsIter(_octalDetectors.begin());
  pixDets_t::const_iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    for (size_t i(0); i< octalDetsIter->tiles.size(); ++i)
    {
      if (cacheTileParams(octalDetsIter->tiles[i], runNbr, blNbr, highTagNbr, *first))
        octalDetsIter->notLoaded = false;
      detTileParams &tileParams(octalDetsIter->tiles[i]);

      /** retrieve the additonal information of the tiles of an octal detector */
      int funcstatus(0);
      /** the position in x in the lab space in um */
      funcstatus = ReadDetPosX(tileParams.posx_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameters: pos X of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      else
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tileParams.name + "' has pos x '" + toString(tileParams.posx_um) +
                 "' um");

      /** the position in y in the lab space in um */
      funcstatus = ReadDetPosY(tileParams.posy_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameters: pos Y of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      else
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tileParams.name + "' has pos y '" + toString(tileParams.posy_um) +
                 "' um");

      /** the position in z in the lab space in um */
      funcstatus = ReadDetPosZ(tileParams.posz_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameter: pos Z of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      else
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tileParams.name + "' has pos z '" + toString(tileParams.posz_um) +
                 "' um");

      /** the angle in degrees in the lab space */
      funcstatus = ReadDetRotationAngle(tileParams.angle_deg,tileParams.name.c_str(),
                                        blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameter: angle of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      else
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tileParams.name + "' has angle '" + toString(tileParams.angle_deg) +
                 "' degrees");

      /** the gain of the detector tile */
      funcstatus = ReadAbsGain(tileParams.gain,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameter: absolute gain of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      else
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tileParams.name + "' has absolute gain '" +
                 toString(tileParams.gain) + "'");

    }
    /** if the tiles of the octal detector should be normalized, calculate the
     *  relative gain of the individual tiles with respect to the first tile
     *  and store the relative gains within the tile
     */
    if (octalDetsIter->normalize)
    {
      detTileParams firstTile(octalDetsIter->tiles[0]);
      firstTile.normalize = false;
      for (size_t j = 1; j<octalDetsIter->tiles.size(); ++j)
      {
        detTileParams &tile(octalDetsIter->tiles[j]);
        tile.normalize = true;
        tile.relativeGain = tile.gain / firstTile.gain;
        Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                 tile.name + "' will be normalized with relative gain of '" +
                 toString(tile.relativeGain) + "'");
      }
    }
  }
}

uint64_t SACLAConverter::operator()(const int runNbr, const int blNbr,
                                    const int highTagNbr, const int tagNbr,
                                    CASSEvent& event)
{
  /** set the event id from the highTag and Tag number */
  event.id() = (static_cast<uint64_t>(highTagNbr)<<32) + tagNbr;
  uint64_t datasize(0);

  /** check if the event contains the machine data container, if so get a
   *  reference to it. Otherwise throw an error.
   */
  if (event.devices().find(CASSEvent::MachineData) == event.devices().end())
    throw runtime_error("SACLAConverter():The CASSEvent does not contain a Machine Data Device");
  MachineDataDevice &md
    (*dynamic_cast<MachineDataDevice*>(event.devices()[CASSEvent::MachineData]));


  /** if requested retrieve the accelarator parameters */
  if (_retrieveAcceleratorData)
  {
    int funcstatus(0);
    double fbuf(0);
    /** electron energy */
    funcstatus = ReadConfigOfElectronEnergy(fbuf, blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve electron energy for tag'" +
               toString(tagNbr) + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()["Acc_electronEnergy_eV"] = fbuf;

    /** k-params */
    funcstatus = ReadConfigOfKPars(fbuf, blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve k params for tag '" +
               toString(tagNbr) + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()["Acc_KParams"] = fbuf;

    /** the set photon energy */
    funcstatus = ReadConfigOfPhotonEnergy(fbuf, blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve set photon energy for tag '" +
               toString(tagNbr) + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()["Acc_PhotonEnergy"] = fbuf;
  }


  /** go through all requested machine data events and retrieve the corresponding
   *  values for the tag
   */
  machineVals_t::const_iterator machineValsIter(_machineVals.begin());
  machineVals_t::const_iterator machineValsEnd(_machineVals.end());
  for (; machineValsIter != machineValsEnd; ++machineValsIter)
  {
    /** rerference to the machine value */
    const MachineValue &mv(*machineValsIter);
    /** check if the cache contains the machine value for the requested tag */
    machineVals_t::value_type::values_t::const_iterator entry(mv.values.find(tagNbr));
    if (entry == mv.values.end())
    {
      Log::add(Log::ERROR,"SACLAConverter: cannot find beamline value '" +
               mv.databaseName + "' for tag '" + toString(tagNbr) +
               "' in cache.");
      continue;
    }
    md.BeamlineData()[mv.cassName] = entry->second;
    datasize += sizeof(double);
  }


  /** retrieve the container for pixel detectors */
  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("SACLAConverter: CASSEvent does not contains a pixeldetector device");
  pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

  /** read the requested pixel detector data */
  pixDets_t::iterator pixelDetsIter(_pixelDetectors.begin());
  pixDets_t::iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
  {
    detTileParams &tile(pixelDetsIter->tiles[0]);
    /** skip if the detector data isn't loaded */
    if (pixelDetsIter->notLoaded)
      continue;

    /** retrieve the right detector from the cassevent and prepare the frame */
    pixeldetector::Detector &det(dev.dets()[pixelDetsIter->CASSID]);
    det.frame().clear();
    det.columns() = tile.xsize;
    det.rows() =  tile.ysize;
    det.frame().resize(det.rows()*det.columns());
    det.id() = event.id();

    /** get information about the tile and the position with the frame */
    tile.start = det.frame().begin();
    md.BeamlineData()[tile.name+"_Width"] = det.columns();
    md.BeamlineData()[tile.name+"_Height"] = det.rows();
    md.BeamlineData()[tile.name+"_PixSize_um"] = tile.pixsize_um;

    /** retrieve the data with the right type */
    switch(tile.type)
    {
      case Sacla_DATA_TYPE_UNSIGNED_SHORT:
        retrieveTileData<uint16_t>(tile, runNbr, blNbr, highTagNbr, tagNbr);
        break;
      case Sacla_DATA_TYPE_FLOAT:
        retrieveTileData<float>(tile, runNbr, blNbr, highTagNbr, tagNbr);
        break;
      case Sacla_DATA_TYPE_INVALID:
      default:
        Log::add(Log::ERROR,"SACLAConverter: Data type of pixel detector '" +
                 tile.name + "' for tag '" + toString(tagNbr) + "' is unkown");
        break;
    }

    /** notice how much data has been retrieved */
    datasize += tile.bytes_retrieved;
  }

  /** read the requested octal detectors */
  pixDets_t::iterator octalDetsIter(_octalDetectors.begin());
  pixDets_t::iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    pixDets_t::value_type &octdet(*octalDetsIter);
    /** skip if the data of the detector isn't loaded */
    if (octdet.notLoaded)
      continue;

    /** retrieve the right detector from the cassevent and reset it */
    pixeldetector::Detector &det(dev.dets()[octdet.CASSID]);
    det.frame().clear();
    det.columns() = 0;
    det.rows() =  0;
    det.id() = event.id();

    /** make the frame big enough to hold the whole detector data */
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
    {
      const detTileParams &tile(octdet.tiles[i]);
      det.frame().resize(det.frame().size() + tile.xsize*tile.ysize);
      det.columns() = tile.xsize;
      det.rows() +=  tile.ysize;
    }

    /** get the parameters of the tiles and remember where to put the tile
     *  within the frame
     */
    size_t currentsize(0);
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
    {
      detTileParams &tile(octdet.tiles[i]);
      md.BeamlineData()[tile.name+"_Width"]      = tile.xsize;
      md.BeamlineData()[tile.name+"_Height"]     = tile.ysize;
      md.BeamlineData()[tile.name+"_PixSize_um"] = tile.pixsize_um;
      md.BeamlineData()[tile.name+"_PosX_um"]    = tile.posx_um;
      md.BeamlineData()[tile.name+"_PosY_um"]    = tile.posy_um;
      md.BeamlineData()[tile.name+"_PosZ_um"]    = tile.posz_um;
      md.BeamlineData()[tile.name+"_Angle_deg"]  = tile.angle_deg;
      md.BeamlineData()[tile.name+"_AbsGain"]    = tile.gain;

      tile.start = (det.frame().begin() + currentsize);
      const size_t npixels(tile.xsize*tile.ysize);
      currentsize += npixels;
    }

    /** retrive the data of the tiles */
#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
    {
      detTileParams &tile(octdet.tiles[i]);
      /** retrieve the data with the right type */
      switch(tile.type)
      {
        case Sacla_DATA_TYPE_FLOAT:
          retrieveTileData<float>(tile, runNbr, blNbr, highTagNbr, tagNbr);
          break;
        case Sacla_DATA_TYPE_INVALID:
        default:
#ifdef _OPENMP
          #pragma omp critical
#endif
          Log::add(Log::ERROR,"SACLAConverter: Data type of octal detector '" +
                   tile.name + "' for tag '" + toString(tagNbr) + "' is unkown");
          break;
      }
    }

    /** gather the size retrieved of all the tiles */
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
      datasize += octdet.tiles[i].bytes_retrieved;
  }

  return datasize;
}

// Copyright (C) 2014 Lutz Foucar

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
#include "machine_device.h"
#include "cass.h"
#include "log.h"
#include "pixeldetector.hpp"

using namespace cass;
using namespace MachineData;
using namespace std;

/** retrieve a pixel detector from the data
 *
 * details
 *
 * @tparam T the type of the detector data
 *
 * @return size of the retrieved data
 * @param frameStart iterator to the first element of the frame
 * @param detName Name of the ocatal detector (tile names will be deferred from it)
 * @param size the number of pixels in the frame
 * @param runNbr the run number of the experiment
 * @param blNbr the Beamline number of the experiment
 * @param highTagNbr the high tag number for the experiment
 * @param tagNbr the tag number that is associated with the requested data
 *
 * @author Lutz Foucar
 */
template <typename T>
uint64_t retrievePixelDet(pixeldetector::frame_t::iterator frameStart,
                          const string &detName, const size_t size,
                          const int runNbr, const int blNbr,
                          const int highTagNbr, const int tagNbr)
{
  /** retrieve the detector data */
  vector<T> buffer(size);
  int funcstatus(0);
  funcstatus = ReadDetData(&buffer.front(),detName.c_str(),
                           blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve data of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }

  /** copy the data of the detector to the cassevent */
  copy(buffer.begin(),buffer.end(),frameStart);

  return size*sizeof(T);
}


/** cache pixel detector parameters
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
bool cacheDetParams(SACLAConverter::detTileParams &tileParams, int runNbr,
                    int blNbr, int highTagNbr, int tagNbr)
{
  int funcstatus(0);
  funcstatus = ReadXSizeOfDetData(tileParams.xsize,tileParams.name.c_str(),
                                  blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: width of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  funcstatus = ReadYSizeOfDetData(tileParams.ysize,tileParams.name.c_str(),
                                  blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: height of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  funcstatus = ReadSizeOfDetData(tileParams.datasize_bytes,tileParams.name.c_str(),
                                 blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: datasize of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  funcstatus = ReadPixelSize(tileParams.pixsize_um,tileParams.name.c_str(),
                             blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParamss: pixelsize of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
  funcstatus = ReadDetDataType(tileParams.type,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"cacheDetParams: datatype of '" +
             tileParams.name + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return false;
  }
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
  }
  s.endArray();

  /** set the requested octal detectors */
  size = s.beginReadArray("DatabaseValues");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string machineValName(s.value("ValueName","Invalid").toString().toStdString());
    /** skip if the value name has not been set */
    if (machineValName != "Invalid")
      _machineVals.insert(make_pair(machineValName,machineVals_t::mapped_type()));
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
    vector<string> machineValueStringList;
    funcstatus = ReadSyncDataList(&machineValueStringList,
                                  const_cast<char*>(machineValsIter->first.c_str()),
                                  highTagNbr,tagList);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"SACLAConverter::cacheParameters could not cache values of '" +
               machineValsIter->first + "' ErrorCode is '" + toString(funcstatus) + "'");
      continue;
    }
    if (machineValueStringList.size() != tagList.size())
    {
      Log::add(Log::ERROR,"SACLAConverter:cacheParameters caching '" +
               machineValsIter->first + "' did not return the right size");
      continue;
    }
    /** put the retrieved values into the cache */
    vector<int>::const_iterator tag(tagList.begin());
    vector<int>::const_iterator tagListEnd(tagList.end());
    vector<string>::const_iterator machine(machineValueStringList.begin());
    for (; tag != tagListEnd; ++tag, ++machine)
      (machineValsIter->second)[*tag] = *machine;
  }

  /** for all pixel dets retrieve the non changing parameters from the first
   *  image
   */
  pixDets_t::iterator pixelDetsIter(_pixelDetectors.begin());
  pixDets_t::const_iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
    if (cacheDetParams(pixelDetsIter->tiles[0],runNbr,blNbr,highTagNbr,*first))
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
      if (cacheDetParams(octalDetsIter->tiles[i], runNbr, blNbr, highTagNbr, *first))
        octalDetsIter->notLoaded = false;
      detTileParams &tileParams(octalDetsIter->tiles[i]);
      /** retrieve the position and tilt of the tile */
      int funcstatus(0);
      funcstatus = ReadDetPosX(tileParams.posx_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"retrieveOctal: pos X of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      funcstatus = ReadDetPosY(tileParams.posy_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"retrieveOctal: pos Y of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      funcstatus = ReadDetPosZ(tileParams.posz_um,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"retrieveOctal: pos Z of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      funcstatus = ReadDetRotationAngle(tileParams.angle_deg,tileParams.name.c_str(),
                                        blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"retrieveOctal: angle of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
      }
      /** get the gain of the detector tile */
      funcstatus = ReadAbsGain(tileParams.gain,tileParams.name.c_str(),
                               blNbr, runNbr, highTagNbr, *first);
      if (funcstatus)
      {
        Log::add(Log::ERROR,"retrieveOctal: absolute gain of '" +
                 tileParams.name + "' for tag '" + toString(*first) +
                 "' ErrorCode is '" + toString(funcstatus) + "'");
        octalDetsIter->notLoaded = true;
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
   *  values for the tag */
  machineVals_t::const_iterator machineValsIter(_machineVals.begin());
  machineVals_t::const_iterator machineValsEnd(_machineVals.end());
  for (; machineValsIter != machineValsEnd; ++machineValsIter)
  {
    /** check if the cache contains the machine value for the requested tag */
    map<int,string>::const_iterator entry(machineValsIter->second.find(tagNbr));
    if (entry == machineValsIter->second.end())
    {
      Log::add(Log::ERROR,"SACLAConverter: cannot find beamline value '" +
               machineValsIter->first + "' for tag '" + toString(tagNbr) +
               "' in cache.");
      continue;
    }
    /** check if retrieved value can be converted to double, and if so add it
     *  to the machine data, otherwise issue an error and continue
     *  @note the retrieved values might contain the unit of the value in the
     *        string, therefore one has to remove all characters from the string
     */
    QString machineValueQString(QString::fromStdString(entry->second));
    machineValueQString.remove(QRegExp("V|C|pulse|a\\.u\\."));
    bool isDouble(false);
    double machineValue(machineValueQString.toDouble(&isDouble));
    if (isDouble)
      md.BeamlineData()[machineValsIter->first] = machineValue;
    else
    {
      Log::add(Log::ERROR,"SACLAConverter: '" + machineValsIter->first +
               "' for tag '" + toString(tagNbr) + "': String '" + entry->second +
               "' which is altered to '" + machineValueQString.toStdString() +
               "' to remove units, cannot be converted to double");
      continue;
    }
    datasize += sizeof(double);
  }


  /** retrieve the container for pixel detectors */
  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("SACLAConverter: CASSEvent does not contains a pixeldetector device");
  pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

  /** read the requested pixel detector data */
  pixDets_t::const_iterator pixelDetsIter(_pixelDetectors.begin());
  pixDets_t::const_iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
  {
    if (pixelDetsIter->notLoaded)
      continue;
    /** retrieve the right detector from the cassevent */
    pixeldetector::Detector &det(dev.dets()[pixelDetsIter->CASSID]);
    det.frame().clear();
    det.columns() = pixelDetsIter->tiles[0].xsize;
    det.rows() =  pixelDetsIter->tiles[0].ysize;
    det.frame().resize(det.rows()*det.columns());
    det.id() = event.id();
    md.BeamlineData()[pixelDetsIter->tiles[0].name+"_Width"] = det.columns();
    md.BeamlineData()[pixelDetsIter->tiles[0].name+"_Height"] = det.rows();
    md.BeamlineData()[pixelDetsIter->tiles[0].name+"_PixSize_um"] = pixelDetsIter->tiles[0].pixsize_um;
    /** retrieve the data with the right type */
    switch(pixelDetsIter->tiles[0].type)
    {
      case Sacla_DATA_TYPE_UNSIGNED_SHORT:
        datasize += retrievePixelDet<uint16_t>(det.frame().begin(),
                                               pixelDetsIter->tiles[0].name,
                                               det.columns()*det.rows(),
                                               runNbr,blNbr,highTagNbr,tagNbr);
        break;
      case Sacla_DATA_TYPE_FLOAT:
        datasize += retrievePixelDet<float>(det.frame().begin(),
                                            pixelDetsIter->tiles[0].name,
                                            det.columns()*det.rows(),
                                            runNbr,blNbr,highTagNbr,tagNbr);
        break;
      case Sacla_DATA_TYPE_INVALID:
      default:
        Log::add(Log::ERROR,"SACLAConverter: Data type of pixel detector '" +
                 pixelDetsIter->tiles[0].name + "' for tag '" + toString(tagNbr) +
                 "' is unkown");
        break;
    }
  }

  /** read the requested octal detectors */
  pixDets_t::const_iterator octalDetsIter(_octalDetectors.begin());
  pixDets_t::const_iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    if (octalDetsIter->notLoaded)
      continue;
    /** retrieve the right detector from the cassevent */
    pixeldetector::Detector &det(dev.dets()[octalDetsIter->CASSID]);
    det.frame().clear();
    det.columns() = 0;
    det.rows() =  0;
    det.id() = event.id();
    float gainRef(0);

    for (size_t i(0); i<octalDetsIter->tiles.size(); ++i)
    {
      det.frame().resize(det.frame().size() +
                         octalDetsIter->tiles[i].xsize*octalDetsIter->tiles[i].ysize);
      det.columns() = octalDetsIter->tiles[i].xsize;
      det.rows() +=  octalDetsIter->tiles[i].ysize;
    }

    size_t currentsize(0);
    for (size_t i(0); i<octalDetsIter->tiles.size(); ++i)
    {
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_Width"] = octalDetsIter->tiles[i].xsize;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_Height"] = octalDetsIter->tiles[i].ysize;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_PixSize_um"] = octalDetsIter->tiles[i].pixsize_um;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_PosX_um"] = octalDetsIter->tiles[i].posx_um;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_PosY_um"] = octalDetsIter->tiles[i].posy_um;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_PosZ_um"] = octalDetsIter->tiles[i].posz_um;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_Angle_deg"] = octalDetsIter->tiles[i].angle_deg;
      md.BeamlineData()[octalDetsIter->tiles[i].name+"_AbsGain"] = octalDetsIter->tiles[i].gain;

      pixeldetector::frame_t::iterator tileStart(det.frame().begin() + currentsize);
      const size_t npixels(octalDetsIter->tiles[i].xsize*octalDetsIter->tiles[i].ysize);
      currentsize += npixels;
      pixeldetector::frame_t::iterator tileEnd(det.frame().begin() + currentsize);
      /** retrieve the data with the right type */
      switch(octalDetsIter->tiles[i].type)
      {
        case Sacla_DATA_TYPE_FLOAT:
          datasize += retrievePixelDet<float>(tileStart,
                                              octalDetsIter->tiles[i].name,
                                              npixels, runNbr,blNbr,highTagNbr,
                                              tagNbr);
          break;
        case Sacla_DATA_TYPE_INVALID:
        default:
          Log::add(Log::ERROR,"SACLAConverter: Data type of octal detector '" +
                   octalDetsIter->tiles[i].name + "' for tag '" + toString(tagNbr) +
                   "' is unkown");
          break;
      }
      /** in an octal MPCCD one needs to leverage the different tiles by its gain,
       *  taking the gain of the first tile as a reference
       */
      if (octalDetsIter->normalize)
      {
        if (i)
        {
          const float gain(octalDetsIter->tiles[i].gain);
          const float relGain(gain / gainRef);
          transform(tileStart,tileEnd, tileStart, bind1st(multiplies<float>(),relGain));
        }
        else
          gainRef = octalDetsIter->tiles[i].gain;
      }
    }
  }

  return datasize;
}

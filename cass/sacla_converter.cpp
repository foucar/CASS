// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_converter.cpp contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <SaclaDataAccessUserAPI.h>

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
 * @param det reference to the detector that should be filled with data
 * @param md reference to the machine data. Needed for additional det information
 * @param detName Name of the ocatal detector (tile names will be deferred from it)
 * @param runNbr the run number of the experiment
 * @param blNbr the Beamline number of the experiment
 * @param highTagNbr the high tag number for the experiment
 * @param tagNbr the tag number that is associated with the requested data
 *
 * @author Lutz Foucar
 */
template <typename T>
uint64_t retrievePixelDet(pixeldetector::Detector &det, MachineDataDevice & md,
                          string detName, int runNbr, int blNbr, int highTagNbr, int tagNbr)
{
  vector<T> buffer;

  /** get the height and the width of the detector */
  int width(0), height(0), funcstatus(0);
  funcstatus = ReadXSizeOfDetData(width,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not width of '" + detName + "' for tag '" + toString(tagNbr)+
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  funcstatus = ReadYSizeOfDetData(height,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not widt Octal: could not height of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  md.BeamlineData()[detName+"_Width"] = width;
  md.BeamlineData()[detName+"_Height"] = height;

  /** retrieve the datasize of the detector tile in bytes */
  int datasize(0);
  funcstatus = ReadSizeOfDetData(datasize,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve datasize of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }

  /** retrieve the position and tilt of the tile */
  float posx(0), posy(0), posz(0), tilt_deg(0);
  funcstatus = ReadDetPosX(posx,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve pos X of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  funcstatus = ReadDetPosY(posy,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet Octal: could not retrieve pos Y of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  funcstatus = ReadDetPosZ(posz,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve pos Z of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  funcstatus = ReadDetRotationAngle(tilt_deg,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve tilt of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  md.BeamlineData()[detName+"_PosX_um"] = posx;
  md.BeamlineData()[detName+"_PosY_um"] = posy;
  md.BeamlineData()[detName+"_PosZ_um"] = posz;
  md.BeamlineData()[detName+"_Angle_deg"] = tilt_deg;

  /** retrieve the pixelsize of the detector tile */
  float pixsize_um(0);
  funcstatus = ReadPixelSize(pixsize_um,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve pixelsize of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  md.BeamlineData()[detName+"_PixSize_um"] = pixsize_um;

  /** get the gain of the detector tile */
  float gain(0);
  funcstatus = ReadAbsGain(gain,detName.c_str(), blNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve absolute gain of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }
  md.BeamlineData()[detName+"_AbsGain"] = gain;

  /** retrieve the detector data */
  buffer.resize(width*height);
  funcstatus = ReadDetData(&buffer.front(),detName.c_str(), blNbr, runNbr, highTagNbr, tagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"retrievePixelDet: could not retrieve data of '" +
             detName + "' for tag '" + toString(tagNbr) +
             "' ErrorCode is '" + toString(funcstatus) + "'");
    return 0;
  }

  /** copy the data of the detector to the cassevent */
  det.columns() = width;
  det.rows() = det.rows() + height;
  int sizebefore(det.frame().size());
  det.frame().resize(sizebefore + width*height);
  pixeldetector::frame_t::iterator tileStart(det.frame().begin() + sizebefore);
  copy(buffer.begin(),buffer.end(),tileStart);

  return datasize;
}

/** retrieve an octal mpccd detector
 *
 * details
 *
 * @tparam T the type of the detector data
 *
 * @return size of the retrieved data
 * @param det reference to the detector that should be filled with data
 * @param md reference to the machine data. Needed for additional det information
 * @param detName Name of the ocatal detector (tile names will be deferred from it)
 * @param normalize flag to tell whether individual tiles should be normalized
 *                  to the first tile
 * @param runNbr the run number of the experiment
 * @param blNbr the Beamline number of the experiment
 * @param highTagNbr the high tag number for the experiment
 * @param tagNbr the tag number that is associated with the requested data
 *
 * @author Lutz Foucar
 */
template <typename T>
uint64_t retrieveOctal(pixeldetector::Detector &det, MachineDataDevice &md,
                       string detName, bool normalize,
                       int runNbr, int blNbr, int highTagNbr, int tagNbr)
{
  float gainRef(0);
  uint64_t datasize(0);

  /** go through all 8 detector tiles of the octal MPCCD */
  for (int i=0; i<8 ; ++i)
  {
    /** generate the name of the current detector tile */
    string detTileName(detName + "-" + toString(i+1));
    /** remember the size of the frame before the current tile is added to the
     *  cassevent, to be able to determin just that next tile of the assembled
     *  image.
     */
    int sizebefore(det.frame().size());

    /** get all the info and the data of the tile */
    datasize += retrievePixelDet<T>(det,md,detTileName,runNbr,blNbr,highTagNbr,tagNbr);

    /** in an octal MPCCD one needs to leverage the different tiles by its gain,
     *  taking the gain of the first tile as a reference
     */
    if (normalize)
    {
      if (i)
      {
        const float gain(md.BeamlineData()[detTileName + "_AbsGain"]);
        const float relGain(gain / gainRef);
        pixeldetector::frame_t::iterator tileStart(det.frame().begin() + sizebefore);
        pixeldetector::frame_t::iterator tileEnd(det.frame().end());
        transform(tileStart,tileEnd, tileStart, bind1st(multiplies<T>(),relGain));
      }
      else
        gainRef = md.BeamlineData()[detTileName + "_AbsGain"];
    }
  }

  return datasize;
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
    int32_t key(s.value("CASSID",0).toInt());
    bool normalize(s.value("NormalizeToAbsGain",true).toBool());
    /** skip if the detector name has not been set */
    if (detID != "Invalid")
      _octalDetectors[key] = make_pair(detID,normalize);
  }
  s.endArray();

  /** set the requested pixel detectors */
  size = s.beginReadArray("PixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string detID(s.value("DetectorIDName","Invalid").toString().toStdString());
    int32_t key(s.value("CASSID",0).toInt());
    /** skip if the detector name has not been set */
    if (detID != "Invalid")
      _pixelDetectors[key] = detID;
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
      _machineVals.push_back(machineValName);
  }
  s.endArray();

  s.endGroup();
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


  /** go through all requested machine data events and retrieve the corresponding *  values for the tag */
  vector<string>::const_iterator machineValsIter(_machineVals.begin());
  vector<string>::const_iterator machineValsEnd(_machineVals.end());
  for (; machineValsIter != machineValsEnd; ++machineValsIter)
  {
    /** retrieve the machine data value as string */
    vector<string> machineValueStringList;
    vector<int>tagNbrList(1,tagNbr);
    int funcstatus(0);
    funcstatus = ReadSyncDataList(&machineValueStringList,const_cast<char*>(machineValsIter->c_str()),
                                  highTagNbr,tagNbrList);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"SACLAConverter: could not retrieve value of '" +
               *machineValsIter + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      continue;
    }
    if (machineValueStringList.size() != 1)
    {
      Log::add(Log::ERROR,"SACLAConverter: retrieval of value of '" +
               *machineValsIter + "' for tag '" + toString(tagNbr) +
               "' did not return the right size");
      continue;
    }
    /** check if retrieved value can be converted to double, and if so add it
     *  to the machine data, otherwise issue an error and continue
     *  @note the retrieved values mgith contain the unit of the value in the
     *        string, therefore one has to remove all characters from the string
     */
    QString machineValueQString(QString::fromStdString(machineValueStringList[0]));
    machineValueQString.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    bool isDouble(false);
    double machineValue(machineValueQString.toDouble(&isDouble));
    if (isDouble)
      md.BeamlineData()[*machineValsIter] = machineValue;
    else
      Log::add(Log::ERROR,"SACLAConverter: '" + *machineValsIter + "' for tag '"
               + toString(tagNbr) + "' string '" + machineValueStringList[0] +
               "' cannot be converted to double");
    datasize += sizeof(double);
  }


  /** retrieve the container for pixel detectors */
  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("pixeldetector::retrieveDet: There is no  pixeldetector device within the CASSEvent");
  pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

  /** read the requested pixel detector data */
  map<int32_t,string>::const_iterator pixelDetsIter(_pixelDetectors.begin());
  map<int32_t,string>::const_iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
  {
    /** determine the detector pixel data type */
    Sacla_DetDataType type;
    if (ReadDetDataType(type,pixelDetsIter->second.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAConverter: could not retrieve data type of '" +
               pixelDetsIter->second + "' for tag '" + toString(tagNbr) + "'");
      continue;
    }
    /** retrieve the right detector from the cassevent */
    pixeldetector::Detector &det(dev.dets()[pixelDetsIter->first]);
    det.frame().clear();
    det.columns() = 0;
    det.rows() =  0;
    det.id() = event.id();
    /** retrieve the data with the right type */
    switch(type)
    {
      case Sacla_DATA_TYPE_UNSIGNED_SHORT:
        datasize += retrievePixelDet<uint16_t>(det,md,pixelDetsIter->second,
                                               runNbr,blNbr,highTagNbr,tagNbr);
        break;
      case Sacla_DATA_TYPE_FLOAT:
        datasize += retrievePixelDet<float>(det,md,pixelDetsIter->second,
                                            runNbr,blNbr,highTagNbr,tagNbr);
        break;
      case Sacla_DATA_TYPE_INVALID:
      default:
        Log::add(Log::ERROR,"SACLAConverter: Data type of pixel detector '" +
                 pixelDetsIter->second + "' for tag '" + toString(tagNbr) +
                 "' is unkown");
        break;
    }
  }

  /** read the requested octal detector data */
  octalDets_t::const_iterator octalDetsIter(_octalDetectors.begin());
  octalDets_t::const_iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    /** determine the detector pixel data type */
    Sacla_DetDataType type;
    if (ReadDetDataType(type,octalDetsIter->second.first.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAConverter: could not retrieve data type of '" +
               octalDetsIter->second.first + "' for tag '" + toString(tagNbr) + "'");
      continue;
    }
    /** retrieve the right detector from the cassevent */
    pixeldetector::Detector &det(dev.dets()[octalDetsIter->first]);
    det.frame().clear();
    det.columns() = 0;
    det.rows() =  0;
    det.id() = event.id();
    /** retrieve the data with the right type */
    switch(type)
    {
      case Sacla_DATA_TYPE_FLOAT:
        datasize += retrieveOctal<float>(det,md,octalDetsIter->second.first,
                                         octalDetsIter->second.second,
                                         runNbr,blNbr,highTagNbr,tagNbr);
        break;
      default:
        Log::add(Log::ERROR,"SACLAConverter: Data type of octal detector '" +
                 octalDetsIter->second.first + "' for tag '" + toString(tagNbr) +
                 "' is unkown");
        break;
    }
  }

  return datasize;
}

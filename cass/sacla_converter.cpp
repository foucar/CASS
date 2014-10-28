// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_converter.cpp contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <SaclaDataAccessUserAPI.h>

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
 * @param blNbr the Beamline number of the experiment
 * @param highTagNbr the high tag number for the experiment
 * @param tagNbr the tag number that is associated with the requested data
 */
template <typename T>
uint64_t retrieveOctal(pixeldetector::Detector &det, MachineDataDevice &md,
                       string detName, bool normalize,
                       int blNbr, int highTagNbr, int tagNbr)
{
  det.frame().clear();
  det.columns() = 0;
  det.rows() =  0;

  vector<T> buffer;
  float gainRef(0);
  uint64_t datasize(0);

  /** go through all 8 detector tiles of the octal MPCCD */
  for (int i=0; i<8 ; ++i)
  {
    /** generate the name of the current detector tile */
    string detTileName(detName + "-" + toString(i+1));

    /** get the height and the width of the detector */
    int width(0), height(0), funcstatus(0);
    funcstatus = ReadXSizeOfDetData(width,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not width of '" +
               detName + "' for tag '" + toString(tagNbr)+
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    funcstatus = ReadYSizeOfDetData(height,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not height of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName+"_Width"] = width;
    md.BeamlineData()[detTileName+"_Height"] = height;

    /** retrieve the datasize of the detector tile in bytes */
    int tiledatasize(0);
    funcstatus = ReadSizeOfDetData(tiledatasize,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve datasize of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    datasize += tiledatasize;

    /** retrieve the position and tilt of the tile */
    float posx(0), posy(0), posz(0), tilt_deg(0);
    funcstatus = ReadDetPosX(posx,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve pos X of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    funcstatus = ReadDetPosY(posy,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve pos Y of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    funcstatus = ReadDetPosZ(posz,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve pos Z of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    funcstatus = ReadDetRotationAngle(tilt_deg,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve tilt of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName+"_PosX_um"] = posx;
    md.BeamlineData()[detTileName+"_PosY_um"] = posy;
    md.BeamlineData()[detTileName+"_PosZ_um"] = posz;
    md.BeamlineData()[detTileName+"_Angle_deg"] = tilt_deg;

    /** retrieve the pixelsize of the detector tile */
    float pixsize_um(0);
    funcstatus = ReadPixelSizeInMicroMeter(pixsize_um,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve pixelsize of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName+"_PixSize_um"] = pixsize_um;

    /** get the gain of the detector tile */
    float gain(0);
    funcstatus = ReadAbsGain(gain,detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve absolute gain of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName+"_AbsGain"] = gain;

    /** detector recording frequency */
    int ibuf(0);
    funcstatus = ReadConfigOfDetRecordFreq(ibuf, const_cast<char*>(detTileName.c_str()),
                                           blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve det recording frequency of det '" +
               detName + "' for tag '" + toString(tagNbr) + "' ErrorCode is '" +
               toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName+"_DetectorRecordingFrequency"] = ibuf;

    /** detector frequency */
    funcstatus = ReadConfigOfDetFreq(ibuf, const_cast<char*>(detTileName.c_str()),
                                     blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve detector frequency of det '" +
               detName + "' for tag '" + toString(tagNbr) + "' ErrorCode is '" +
               toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[detTileName + "_DetectorFrequency"] = ibuf;

    /** retrieve the detector data */
    buffer.resize(width*height);
    funcstatus = ReadDetData(&buffer.front(),detTileName.c_str(), blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve data of '" +
               detName + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }

    /** copy the data of the detector to the assembled image */
    det.columns() = width;
    det.rows() = det.rows() + height;
    int sizebefore(det.frame().size());
    det.frame().resize(sizebefore + width*height);
    pixeldetector::frame_t::iterator tileStart(det.frame().begin() + sizebefore);
    copy(buffer.begin(),buffer.end(),tileStart);

    /** in an octal MPCCD one needs to leverage the different tiles by its gain,
     *  taking the gain of the first tile as a reference
     */
    if (normalize)
    {
      if (i)
      {
        const float relGain(gain / gainRef);
        pixeldetector::frame_t::iterator tileEnd(det.frame().end());
        transform(tileStart,tileEnd, tileStart, bind1st(multiplies<T>(),relGain));
      }
      else
        gainRef = gain;
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

uint64_t SACLAConverter::operator()(const int blNbr, const int highTagNbr,
                                    const int tagNbr, CASSEvent& event)
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
    int ibuf(0);
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

    /** sacla frequency */
    funcstatus = ReadConfigOfSACLAFreq(ibuf, blNbr, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve sacla frequency for tag '" +
               toString(tagNbr) + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()["Acc_SACLAFrequency"] = ibuf;

    /** master frequency */
    funcstatus = ReadConfigOfMasterFreq(ibuf, highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve master frequency for tag '" +
               toString(tagNbr) + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()["Acc_MasterFrequency"] = ibuf;
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
    /** sync data frequency */
    int ibuf(0);
    funcstatus = ReadConfigOfSyncDataFreq(ibuf, const_cast<char*>(machineValsIter->c_str()),
                                          highTagNbr, tagNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve sync data frequency of '" +
               *machineValsIter + "' for tag '" + toString(tagNbr) +
               "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    md.BeamlineData()[*machineValsIter + "_SyncDataFrequency"] = ibuf;
    /** check if retrieved value can be converted to double, and if so add it
     *  to the machine data, otherwise issue an error and continue
     */
    bool isDouble(false);
    QString machineValueQString(QString::fromStdString(machineValueStringList[0]));
    double machineValue(machineValueQString.toDouble(&isDouble));
    if (isDouble)
      md.BeamlineData()[*machineValsIter] = machineValue;
    else
      Log::add(Log::ERROR,"SACLAConverter: '" + *machineValsIter + "' for tag '"
               + toString(tagNbr) +
               "' doesn't contain a string that can be converted to double");
    datasize += sizeof(double);
  }


  /** read the requested octal detector data */
  /** retrieve the container for pixel detectors */
  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("pixeldetector::retrieveDet: There is no  pixeldetector device within the CASSEvent");
  pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

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
    det.id() = event.id();

    /** retrieve the data with the right type */
    switch(type)
    {
      case Sacla_DATA_TYPE_FLOAT:
        datasize += retrieveOctal<float>(det,md,octalDetsIter->second.first,
                                         octalDetsIter->second.second,
                                         blNbr,highTagNbr,tagNbr);
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

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
 *  details
 *
 * @tparam T the type of the detector data
 *
 * @return size of the retrieved data
 * @param det
 * @param detName
 * @param blNbr
 * @param highTagNbr
 * @param tagNbr
 */
template <typename T>
uint64_t retrieveOctal(pixeldetector::Detector &det, string detName,
                   int blNbr, int highTagNbr, int tagNbr)
{
  vector<T> buffer;
  det.frame().resize(512*1024*8);
  det.columns() = 512;
  det.rows() = 8*1024;

  float gainRef=0;
  for (int i=0; i<8 ; ++i)
  {
    string detTileName(detName + "-" + toString(i+1));
    int width=0, height=0;
    if (ReadXSizeOfDetData(width,detTileName.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not width of '" +
               detName + "' for tag '" + toString(tagNbr) + "'");
      return 0;
    }
    if (ReadYSizeOfDetData(height,detTileName.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not height of '" +
               detName + "' for tag '" + toString(tagNbr) + "'");
      return 0;
    }
    buffer.resize(width*height);
    if (ReadDetData(&buffer.front(),detTileName.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve data of '" +
               detName + "' for tag '" + toString(tagNbr) + "'");
      return 0;
    }
    float gain(0);
    if (ReadAbsGain(gain,detTileName.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"retrieve Octal: could not retrieve absolute gain of '" +
               detName + "' for tag '" + toString(tagNbr) + "'");
      return 0;
    }

    pixeldetector::frame_t::iterator tileStart(det.frame().begin() + i*512*1024);
    copy(buffer.begin(),buffer.end(),tileStart);

    if (i)
    {
      const float relGain(gain / gainRef);
      pixeldetector::frame_t::iterator tileEnd(det.frame().begin() + (i+1)*512*1024);
      transform(tileStart,tileEnd, tileStart, bind1st(multiplies<float>(),relGain));
    }
    else
      gainRef = gain;
  }
  return det.frame().size() * sizeof(float);
}


SACLAConverter::SACLAConverter()
{}

void SACLAConverter::loadSettings()
{
  CASSSettings s;
  s.beginGroup("SACLAConverter");

  /** set the requested octal detectors */
  int size = s.beginReadArray("OctalPixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string detID(s.value("DetectorIDName","Invalid").toString().toStdString());
    int32_t key(s.value("CASSID",0).toInt());

    /** skip if the detector name has not been set */
    if (detID == "Invalid")
      continue;

    _octalDetectors[key] = detID;
  }
  s.endArray();

  /** set the requested octal detectors */
  size = s.beginReadArray("MachineParameters");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string machineValName(s.value("ValueName","Invalid").toString().toStdString());

    /** skip if the value name has not been set */
    if (machineValName == "Invalid")
      continue;

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
  /** go through all requested machine data events and retrieve the corresponding *  values for the tag */
  vector<string>::const_iterator machineValsIter(_machineVals.begin());
  vector<string>::const_iterator machineValsEnd(_machineVals.end());
  for (; machineValsIter != machineValsEnd; ++machineValsIter)
  {
    /** retrieve the machine data value as string */
    vector<string> machineValueStringList;
    vector<int>tagNbrList(1,tagNbr);
    if (ReadSyncDataList(&machineValueStringList,const_cast<char*>(machineValsIter->c_str()),
                         highTagNbr,tagNbrList) != 0)
    {
      Log::add(Log::ERROR,"SACLAConverter: could not retrieve value of '" +
               *machineValsIter + "' for tag '" + toString(tagNbr) + "'");
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

  map<int32_t,string>::const_iterator octalDetsIter(_octalDetectors.begin());
  map<int32_t,string>::const_iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    /** determine the detector pixel data type */
    Sacla_DetDataType type;
    if (ReadDetDataType(type,octalDetsIter->second.c_str(), blNbr, highTagNbr, tagNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAConverter: could not retrieve data type of '" +
               octalDetsIter->second + "' for tag '" + toString(tagNbr) + "'");
      continue;
    }

    /** retrieve the right detector from the cassevent */
    pixeldetector::Detector &det(dev.dets()[octalDetsIter->first]);
    det.id() = event.id();

    /** retrieve the data with the right type */
    switch(type)
    {
      case Sacla_DATA_TYPE_FLOAT:
        datasize += retrieveOctal<float>(det,octalDetsIter->second,
                                         blNbr,highTagNbr,tagNbr);
        break;
      default:
        Log::add(Log::ERROR,"SACLAConverter: Data type of octal detector '" +
                 octalDetsIter->second + "' for tag '" + toString(tagNbr) +
                 "' is unkown");
        break;
    }

  }

  return datasize;

//  string line;
//  vector<double> values;
//  while(true)
//  {
//    getline(file, line);
//    values.clear();
//    _split(line,values,_delim);
//    if (!values.empty())
//      break;
//  }
//  if(_headers.size() != values.size())
//    throw runtime_error("TextReader(): In file '" + _filename +
//                        "' are not enough values for the amount of values '" + toString(values.size()) +
//                        "' suggested by the header '" + toString(_headers.size())+
//                        "'. This is the line: " + line);
//
//  if (event.devices().find(CASSEvent::MachineData) == event.devices().end())
//    throw runtime_error("TextReader():The CASSEvent does not contain a Machine Data Device");
//
//  MachineDataDevice &md
//    (*dynamic_cast<MachineDataDevice*>(event.devices()[CASSEvent::MachineData]));
//
//  vector<double>::const_iterator value(values.begin());
//  vector<string>::const_iterator head(_headers.begin());
//  for (;value != values.end(); ++value, ++head)
//    md.BeamlineData()[*head] = *value;
//
//  event.id() = md.BeamlineData()[_eventIdhead];
//
}

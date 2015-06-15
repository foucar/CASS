// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_online_input.cpp contains input that uses sacla as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "sacla_online_input.h"

#include <OnlineUserAPI.h>

#include "cass_settings.h"
#include "cass_exceptions.h"
#include "log.h"
#include "sacla_converter.h"
#include "pixeldetector.hpp"
#include "machine_device.h"

using namespace cass;
using namespace std;

namespace cass
{

/** A Tile of an octal Detector
 *
 * @author Lutz Foucar
 */
struct DetectorTile
{
  /** constructor
   *
   *  connects to the right socket and retrieves all non changing parameters
   *
   * @param name The name of the detector tile
   */
  DetectorTile(const string &name)
    : _name(name)
  {
    int funcstatus(0);
    /** get the socketID of the requested detector */
    funcstatus = ol_connect(_name.c_str(), &_sockID);
    if (funcstatus < 0)
    {
      Log::add(Log::ERROR,"DetectorTile: could not retrieve socket id of '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return;
    }

    /** get the size of the data and the needed worksize */
    int datasize = 0, worksize = 0;
    funcstatus = ol_getDataSize(_sockID, &datasize, &worksize);
    if (funcstatus < 0)
    {
      Log::add(Log::ERROR,"DetectorTile: could not retrieve datasize'" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return;
    }
    _databuffer.resize(datasize);
    _workbuffer.resize(worksize);

    /** retrieve detector data once to retrieve all non changing parameters */
    retrieveData();

    /** read the width and height of the detector */
    funcstatus = ol_readDetSize(&_databuffer.front(), &xsize, &ysize);
    if (funcstatus < 0)
    {
      Log::add(Log::ERROR,"DetectorTile: could not extract width and height of detector '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return;
    }
    /** read the gain of the detector tile */
    funcstatus = ol_readAbsGain(&_databuffer.front(), &gain);
    if (funcstatus < 0)
    {
      Log::add(Log::ERROR,"DetectorTile: could not extract the gain of detector '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return;
    }
  }

  /** read the data
   *
   * defaulty read the latest tag (tag == -1). If tag is given, it will read
   * data associated with the requested tag.
   *
   * @param tag the tag that the data should be read for
   */
  void retrieveData(int tag=-1)
  {
    int outputTag = 0;
    int funcstatus = ol_collectDetData(_sockID, tag, &_databuffer.front(),
                                       _databuffer.size(), &_workbuffer.front(),
                                       _workbuffer.size(), &outputTag);
    if (funcstatus < 0)
      Log::add(Log::ERROR,"DetectorTile: could not retrieve data of detector '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
  }

  /** copy the tile data to the frame
   *
   * @return the size of the data copied in bytes
   * @param frameStart iterator that points to the first point of the tile in
   *                   the frame
   */
  uint64_t copyData(pixeldetector::frame_t::iterator frameStart)
  {
    /** retrieve the data from the databuffer */
    float *data_org(0);
    int funcstatus = ol_readDetData(&_databuffer.front(), &data_org);
    if (funcstatus < 0 || !data_org)
    {
      Log::add(Log::ERROR,"SACLAOnlineInput: could not extract data of detector '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    /** copy the tile data to the frame */
    copy(data_org, data_org+xsize*ysize, frameStart);

    return xsize*ysize*sizeof(float);
  }

  /** the latest Tag read from the current databuffer
   *
   * @return the latest Tag
   */
  int latestTag()
  {
    int tag(0);
    int funcstatus = ol_readTagNum(&_databuffer.front(), &tag);
    if (funcstatus < 0)
      Log::add(Log::ERROR,"DetectorTile::latestTag could not extract the tag from '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
    return tag;
  }

  /** the latest Run Number read from the current databuffer
   *
   * @return latest run number
   */
  int latestRun()
  {
    int run(0);
    int funcstatus = ol_readRunNum(&_databuffer.front(), &run);
    if (funcstatus < 0)
      Log::add(Log::ERROR,"DetectorTile::latestRun could not extract the run from '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
    return run;
  }

  /** the name of the detector tile */
  std::string _name;

  /** the gain of the tile */
  float gain;

  /** the width of the tile */
  int xsize;

  /** the height of the tile */
  int ysize;

//  int datasize_bytes;
//    float pixsize_um;
//    float posx_um;
//    float posy_um;
//    float posz_um;
//    float angle_deg;
  Sacla_DetDataType type;

private:
  /** the socket ID to connect to the online API */
  int _sockID;

  /** the buffer with the detector data */
  vector<char> _databuffer;

  /** buffer needed for the retrieval function to work */
  vector<char> _workbuffer;
};// end class DetectorTile


/** An Octal Detector (with 8 Detector Tiles)
 *
 * @author Lutz Foucar
 */
struct OctalDetector
{
  /** get the latest Tag
   *
   * get the latest tag from the first tile
   *
   * @return the latest available tag
   */
  int latestTag()
  {
    tiles.front().retrieveData();
    return tiles.front().latestTag();
  }

  /** get the latest Run Number
   *
   * get the latest runnumber for the first tile
   *
   * @return the latest runnumber
   */
  int latestRun()
  {
    return tiles.front().latestRun();
  }

  /** copy data associtated with the tag to the device of the cassevent
   *
   * @return size of the data that has been retrieved in bytes
   * @param dev reference to the pixeldetector device in the cassevent
   * @param tag the Tag for which the data should be copied
   */
  uint64_t copyData(pixeldetector::Device &dev, int tag)
  {
    /** retrieve the right detector from the cassevent and reset it */
    pixeldetector::Detector &det(dev.dets()[CASSID]);
    det.frame().clear();
    det.columns() = 0;
    det.rows() =  0;
    det.id() = tag;

    /** set the sizes of the frame to fit the whole detector into it */
    for (size_t i(0); i < tiles.size(); ++i)
    {
      det.frame().resize(det.frame().size() +
                         tiles[i].xsize*tiles[i].ysize);
      det.columns() = tiles[i].xsize;
      det.rows() +=  tiles[i].ysize;
    }

    /** initialize some parameters */
    uint64_t datasize(0);
    uint64_t currentsize(0);

    /** copy the data of the tiles to the detector */
    for (size_t i(0); i < tiles.size(); ++i)
    {
      /** if the data for the tag has not been retrieved, retrieve it at this point */
      if (tiles[i].latestTag() != tag)
        tiles[i].retrieveData(tag);

      /** determine where the tile will be copied to in the frame */
      pixeldetector::frame_t::iterator tileStart(det.frame().begin() + currentsize);
      const size_t npixels(tiles[i].xsize*tiles[i].ysize);
      currentsize += npixels;
      pixeldetector::frame_t::iterator tileEnd(det.frame().begin() + currentsize);

      /** copy the data in the tile to the frame */
      datasize += tiles[i].copyData(tileStart);

      /** in an octal MPCCD one needs to leverage the different tiles by its gain,
       *  taking the gain of the first tile as a reference
       */
      if (normalize && i)
      {
        const float relGain(tiles[i].gain / tiles[0].gain);
        transform(tileStart, tileEnd, tileStart, bind1st(multiplies<float>(),relGain));
      }
    }

    return datasize;
  }

  vector<DetectorTile> tiles;
  bool normalize;
  int CASSID;
}; // end struct octal detector

/** a Machine value
 *
 * @author Lutz Foucar
 */
struct MachineValue
{
  /** constructor
   *
   * retrieve the hightag using the provided runnumber
   *
   * @param name The name of this Machine Value
   * @param runNbr The run number with which we can retrieve the high tag
   * @param blNbr The beamline number used to retrieve the right high tag number
   */
  MachineValue(const string &name, int runNbr, int blNbr)
    : cassname(name),
      _name(name),
      _highTagNbr(0)
  {
    int funcstatus,startTagNbr = 0;
    funcstatus = ReadStartTagNumber(_highTagNbr,startTagNbr,blNbr,runNbr);
    if (funcstatus)
      Log::add(Log::ERROR,"MachineValue: could not retrieve hight tag of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) +
               "' Errorcode is '" + toString(funcstatus) + "'");
  }

  /** copy the data corresponding data to the machine device
   *
   * The machine data can only be retrieved using the offline api
   *
   * @return the size of the data that has been copied
   * @param md reference to the machine data devices
   * @param tag the tag for which the data should be retrieved
   */
  uint64_t copyData(MachineData::MachineDataDevice &md, int tag)
  {
    /** retrieve the machinevalue and check if it was retrieved ok */
    vector<string> machineValueStringList;
    vector<int> tagList(1,tag);
    int funcstatus = ReadSyncDataList(&machineValueStringList,
                                      const_cast<char*>(_name.c_str()),
                                      _highTagNbr,tagList);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"MachineValue::copyData could not extract machine values of '" +
               _name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    if (machineValueStringList.size() != tagList.size())
    {
      Log::add(Log::ERROR,"MachineValue:copyData '" +
               _name + "' did not return the right size");
      return 0;
    }
    /** check if retrieved value can be converted to double, and if so add it
     *  to the machine data, otherwise issue an error and continue
     *  @note the retrieved values might contain the unit of the value in the
     *        string, therefore one has to remove all characters from the string
     */
    QString machineValueQString(QString::fromStdString(machineValueStringList.back()));
    machineValueQString.remove(QRegExp("V|C|pulse|a\\.u\\."));
    bool isDouble(false);
    double machineValue(machineValueQString.toDouble(&isDouble));
    if (isDouble)
      md.BeamlineData()[cassname] = machineValue;
    else
    {
      Log::add(Log::ERROR,"MachineValue::copyData '" + _name + "' for tag '" +
               toString(tag) + "': String '" + machineValueStringList.back() +
               "' which is altered to '" + machineValueQString.toStdString() +
               "' to remove units, cannot be converted to double");
      return 0;
    }
    return sizeof(double);
  }

  /** the name of the machine value within the cassevent */
  string cassname;

private:
  /** the name of the Machine value */
  string _name;

  /** the high tag number */
  int _highTagNbr;
};//end struct MachineValue

} //end namespace cass

void SACLAOnlineInput::instance(RingBuffer<CASSEvent> &buffer,
                                Ratemeter &ratemeter,
                                Ratemeter &loadmeter,
                                QObject *parent)
{
  if(_instance)
    throw logic_error("SACLAOnlineInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new SACLAOnlineInput(buffer,ratemeter,loadmeter,parent));
}

SACLAOnlineInput::SACLAOnlineInput(RingBuffer<CASSEvent> &ringbuffer,
                                   Ratemeter &ratemeter,
                                   Ratemeter &loadmeter,
                                   QObject *parent)
  :InputBase(ringbuffer,ratemeter,loadmeter,parent)
{
  Log::add(Log::VERBOSEINFO, "SACLAOnlineInput:: constructed");
}

void SACLAOnlineInput::run()
{
  /** load info about what the user is interested in */
  CASSSettings s;
  s.beginGroup("SACLAOnlineInput");

  /** get the beamline number of the beamline we're running on */
  int BeamlineNbr = s.value("BeamlineNumber",3).toInt();

  /** load requested octal detectors */
  vector<OctalDetector> octalDetectors;
  int size = s.beginReadArray("OctalPixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string detID(s.value("DetectorIDName","Invalid").toString().toStdString());
    /** skip if the detector name has not been set */
    if (detID == "Invalid")
      continue;
    /** load the infos for the detector */
    octalDetectors.push_back(OctalDetector());
    octalDetectors.back().CASSID = s.value("CASSID",0).toInt();
    octalDetectors.back().normalize = s.value("NormalizeToAbsGain",true).toBool();
    /** setup the individual tiles of the detector */
    for (size_t i(0); i<s.value("NbrOfTiles",8).toUInt(); ++i)
      octalDetectors.back().tiles.push_back(DetectorTile(detID + "-" + toString(i+1)));
  }
  s.endArray();

  /** set the requested octal detectors */
  size = s.beginReadArray("DatabaseValues");
  vector<MachineValue> machineValues;
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string machineValName(s.value("ValueName","Invalid").toString().toStdString());
    string cassValName(s.value("CASSName","Invalid").toString().toStdString());
    /** skip if the value name has not been set and not at least one octal detector exitst
     *  @note the octal detector is needed in order to get the run number with
     *        which one can get the high tag number, needed to retrieve the
     *        machine data. Since the machine data needs to be retrieved using
     *        the offline api.
     */
    if (machineValName != "Invalid")
      machineValues.push_back(MachineValue(machineValName,
                                           octalDetectors.back().latestRun(),
                                           BeamlineNbr));
    /** if the cass name is set then overwrite it withing the machinevalue */
    if (cassValName != "Invalid")
      machineValues.back().cassname = cassValName;
  }
  s.endArray();

  s.endGroup();

  /** run until it is quitted */
  Log::add(Log::DEBUG0,"SACLAOnlineInput::run(): starting loop");
  int lastTag(0);
  while(_control != _quit)
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** retrieve a new element from the ringbuffer */
    rbItem_t rbItem(_ringbuffer.nextToFill());
    CASSEvent &evt(*rbItem->element);
    uint64_t datasize(0);

    /** get the part where the detector will be store in from the event */
    CASSEvent::devices_t &devices(evt.devices());
    CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
    if(devIt == devices.end())
      throw runtime_error("SACLAOnlineInput: CASSEvent does not contains a pixeldetector device");
    pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

    /** get the latest tag from one of the octal detectors */
    int latestTag(octalDetectors.front().latestTag());
    if (latestTag > lastTag)
    {
      /** set the event id */
      evt.id() = latestTag;

      /** copy octal detector data to cassevent */
      vector<OctalDetector>::iterator octIter(octalDetectors.begin());
      vector<OctalDetector>::iterator octEnd(octalDetectors.end());
      for(; octIter != octEnd; ++octIter)
      {
        datasize += octIter->copyData(dev,latestTag);
      }


      /** get refrence to the machine device of the CASSEvent */
      CASSEvent::devices_t::iterator mdIt (devices.find(CASSEvent::MachineData));
      if (mdIt == devices.end())
        throw runtime_error("SACLAOnlineInput():The CASSEvent does not contain a Machine Data Device");
      MachineData::MachineDataDevice &md(*dynamic_cast<MachineData::MachineDataDevice*>(mdIt->second));

      /** retrieve requested machinedata and copy it to the CASSEvent*/
      vector<MachineValue>::iterator machIter(machineValues.begin());
      vector<MachineValue>::iterator machEnd(machineValues.end());
      for(; machIter != machEnd; ++machIter)
      {
        datasize += machIter->copyData(md,latestTag);
      }


      /** remember the latest tag */
      latestTag = lastTag;
    }

    /** let the ratemeter know that we're done with another event of with size
     *  datasize and return the element to the ringbuffer, telling it whether it contains
     *  valuable information (datasize is non zero)
     */
    if (!datasize)
      Log::add(Log::WARNING,"SACLAOnlineInput: Event with id '"+
               toString(rbItem->element->id()) + "' is bad: skipping Event");
    newEventAdded(datasize);
    _ringbuffer.doneFilling(rbItem, datasize);
  }
  Log::add(Log::DEBUG0,"SACLAOnlineInput::run(): quitting loop");



  int funcstatus(0);
  string detName("");

  /** get the socketID of the requested detector */
  int sockID = 0;
  funcstatus = ol_connect(detName.c_str(), &sockID);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not retrieve socket id of '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

  /** get the size of the data and the needed worksize */
  int datasize = 0, worksize = 0;
  funcstatus = ol_getDataSize(sockID, &datasize, &worksize);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not retrieve datasize'" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }
  vector<char> pDataStBuf(datasize);
  vector<char> pWorkBuf(worksize);

  /** retrieve data of detector */
  int tag = 0;
  funcstatus = ol_collectDetData(sockID, -1, &pDataStBuf.front(), datasize,
                                 &pWorkBuf.front(), worksize, &tag);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not data of detector '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

  /** retrieve the data from the workspace */
  float *data_org(0);
  funcstatus = ol_readDetData(&pDataStBuf.front(), &data_org);
  if (funcstatus < 0 || !data_org)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not extract data of detector '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

  /** read the width and height of the detector */
  int orgW = 0, orgH = 0;
  funcstatus = ol_readDetSize(&pDataStBuf.front(), &orgW, &orgH);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not extract width and height of detector '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

  /** read the run number */
  int run(0);
  funcstatus = ol_readRunNum(&pDataStBuf.front(), &run);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not extract the runnumber '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

  /** read the gain of the detector tile */
  float gain(0.f);
  funcstatus = ol_readAbsGain(&pDataStBuf.front(), &gain);
  if (funcstatus < 0)
  {
    Log::add(Log::ERROR,"SACLAOnlineInput: could not extract the gain of detector '" +
             detName + "' ErrorCode is '" + toString(funcstatus) + "'");
    return;
  }

}


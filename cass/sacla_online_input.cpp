// Copyright (C) 2015 Lutz Foucar

/**
 * @file sacla_online_input.cpp contains input that uses sacla as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <tr1/functional>

#include "sacla_online_input.h"

#include <OnlineUserAPI.h>

#include "cass_settings.h"
#include "cass_exceptions.h"
#include "log.h"
#include "sacla_converter.h"
#include "pixeldetector.hpp"
#include "machine_device.h"
#include "cass_exceptions.h"

using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;

namespace cass
{

/** A Tile of a Detector
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
    : name(name),
      gain(0.f),
      xsize(0),
      ysize(0),
      datasize(0),
      normalizeID(-1),
      relativeGain(1.f)
  {
    int funcstatus(0);
    /** get the socketID of the requested detector */
    funcstatus = ol_connect(name.c_str(), &_sockID);
    if (funcstatus < 0)
      throw runtime_error("DetectorTile: could not retrieve socket id of '" +
                          name + "' ErrorCode is '" + toString(funcstatus) +
                          "'");

    /** get the size of the data and the needed worksize */
    int datasize = 0, worksize = 0;
    funcstatus = ol_getDataSize(_sockID, &datasize, &worksize);
    if (funcstatus < 0)
      throw runtime_error("DetectorTile: could not retrieve datasize'" +
                          name + "' ErrorCode is '" + toString(funcstatus) +
                           "'");
    _databuffer.resize(datasize,0);
    _workbuffer.resize(worksize,0);

    /** retrieve detector data once to retrieve all non changing parameters */
    retrieveData();

    /** read the width and height of the detector */
    funcstatus = ol_readDetSize(&_databuffer.front(), &xsize, &ysize);
    if (funcstatus < 0)
      throw runtime_error("DetectorTile: could not extract shape of tile '" +
                          name + "' ErrorCode is '" + toString(funcstatus) + "'");

    /** read the gain of the detector tile */
    funcstatus = ol_readAbsGain(&_databuffer.front(), &gain);
    if (funcstatus < 0)
      throw runtime_error("DetectorTile: could not extract gain of tile '" +
                          name + "' ErrorCode is '" + toString(funcstatus) +
                          "'");
  }

  /** read the data
   *
   * defaulty read the latest tag (tag == -1). If tag is given, it will read
   * data associated with the requested tag.
   *
   * @throws TagOutdated when the funcstatus is indicating an outdated tag
   * @throws runtime_error when the funcstatus is an error other than outdated
   *         tag
   *
   * @return the funcstatus when it says outdated tag or no error
   * @param tag the tag that the data should be read for
   */
  void retrieveData(int tag=-1)
  {
    int outputTag(0);
    int funcstatus = ol_collectDetData(_sockID, tag, &_databuffer.front(),
                                       _databuffer.size(), &_workbuffer.front(),
                                       _workbuffer.size(), &outputTag);
    if (funcstatus == -10000)
      throw TagOutdated("DetectorTile: tile '" + name + "': tag '" + toString(tag) +
                         "' on socket '" + toString(_sockID) +
                         "' isn't available anymore");
    if (funcstatus < 0)
      throw runtime_error("DetectorTile: could not retrieve data of tile '" +
                          name + "' for tag '" + toString(tag) +
                          "' using socket '" + toString(_sockID) +
                          "'. ErrorCode is '" + toString(funcstatus) + "'");
  }

  /** copy the tile data to the frame
   *
   * Collect the data from the server to the buffer and copy the tile's data
   * to the frame.
   *
   * @param tag the tag for which to copy the tile data.
   */
  void copyData(int tag)
  {
    /** reset the datasize */
    datasize = 0;

    /** if the data for the tag has not been retrieved, retrieve it at this point */
    if (this->tag() != tag)
      retrieveData(tag);

    /** retrieve pointer to the tile data from the databuffer */
    float *data_org(0);
    int funcstatus = ol_readDetData(&_databuffer.front(), &data_org);
    if (funcstatus < 0 || !data_org)
    {
      Log::add(Log::ERROR,"SACLAOnlineInput: could not extract data of detector '" +
               name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return;
    }

    /** if tile should be normalized, use transform to copy the data, otherwise
     *  just copy the tile data to the frame
     */
    if (normalizeID != -1)
      transform(data_org, data_org+xsize*ysize, start,
                bind1st(multiplies<float>(),relativeGain));
    else
      copy(data_org, data_org+xsize*ysize, start);

    /** set the datasize to the right size */
    datasize = xsize * ysize * sizeof(uint16_t);
  }

  /** the current Tag read from the current databuffer
   *
   * @return the current Tag
   */
  int tag()
  {
    int tag(0);
    int funcstatus = ol_readTagNum(&_databuffer.front(), &tag);
    if (funcstatus < 0)
      Log::add(Log::ERROR,"DetectorTile::latestTag could not extract the tag from '" +
               name + "' ErrorCode is '" + toString(funcstatus) + "'");
    return tag;
  }

  /** the current Run Number read from the current databuffer
   *
   * @return latest run number
   */
  int runNumber()
  {
    int run(0);
    int funcstatus = ol_readRunNum(&_databuffer.front(), &run);
    if (funcstatus < 0)
      Log::add(Log::ERROR,"DetectorTile::latestRun could not extract the run from '" +
               name + "' ErrorCode is '" + toString(funcstatus) + "'");
    return run;
  }

  /** the name of the detector tile */
  std::string name;

  /** the gain of the tile */
  float gain;

  /** the width of the tile */
  int xsize;

  /** the height of the tile */
  int ysize;

  /** the size of the retrieved data */
  uint64_t datasize;

  /** start position of the tile within the frame */
  pixeldetector::frame_t::iterator start;

  /** end position of the tile within the frame */
  pixeldetector::frame_t::iterator end;

  /** id of tile to normalize to */
  int normalizeID;

  /** the relative gain to normalize for */
  float relativeGain;

//  int datasize_bytes;
//  float pixsize_um;
//  float posx_um;
//  float posy_um;
//  float posz_um;
//  float angle_deg;
//  Sacla_DetDataType type;

private:
  /** the socket ID to connect to the online API */
  int _sockID;

  /** the buffer with the detector data */
  vector<char> _databuffer;

  /** buffer needed for the retrieval function to work */
  vector<char> _workbuffer;
};// end class DetectorTile


/** An Octal Detector
 *
 * A detector with a user chosen amount of equal tiles
 *
 * @author Lutz Foucar
 */
struct OctalDetector
{
  /** get the latest Tag
   *
   * if the last Tag is set return the next tag that should be there
   * otherwise retrieve the latest tag and return it
   * @note this hack is needed as trying to retrieve the latest tag
   *       is currently very slow. Once this issue is fixed one can
   *       think of just using the function to retrieve the latest tag.
   *
   * @return the latest available tag
   * @param lastTag the last tag that was valid. If 0 the latest tag will
   *                retrieved otherwise this number will be increased
   *                by tagAdvance
   */
  int latestTag(int lastTag)
  {
    if (lastTag)
      return lastTag + tagAdvance;
    else
    {
      tiles.front().retrieveData();
      return tiles.front().tag();
    }
  }

  /** get the latest Run Number
   *
   * get the latest runnumber for the first tile
   *
   * @return the latest runnumber
   */
  int runNumber()
  {
    return tiles.front().runNumber();
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

    /** resize the frame to fit all tiles into it */
    for (size_t i(0); i < tiles.size(); ++i)
    {
      det.frame().resize(det.frame().size() +
                         tiles[i].xsize*tiles[i].ysize);
      det.columns() = tiles[i].xsize;
      det.rows() +=  tiles[i].ysize;
    }

    /** set where the individual tiles will start and end within the frame */
    size_t currentsize(0);
    for (size_t i(0); i < tiles.size(); ++i)
    {
      tiles[i].start = det.frame().begin() + currentsize;
      const size_t npixels(tiles[i].xsize*tiles[i].ysize);
      currentsize += npixels;
      tiles[i].end = det.frame().begin() + currentsize;
    }

    /** copy the data in the tile to the frame
     *  @note need to use open mp to parallelize since, the tiles vector is too small
     *        to be parallelized automatically by __gnu_parallelize
     *  @note when compiling with openmp one needs to take special care with the
     *        exceptions. They need to be catched within the thread they have been
     *        thrown. To work around this a global exception exists that will be
     *        filled with the exception thrown. After the execution of the
     *        threads it will be checked if the global exeption has been set and
     *        if so, it will be thrown in the main thread. To be catched at a
     *        convenient time.
     */
//    for_each(tiles.begin(), tiles.end(), bind(&DetectorTile::copyData,_1,tag));
#ifdef _OPENMP
    TagOutdated error("",false);
    #pragma omp parallel for shared(error)
#endif
    for (size_t i = 0; i < tiles.size(); ++i)
    {
#ifdef _OPENMP
      try
      {
#endif
        tiles[i].copyData(tag);
#ifdef _OPENMP
      }
      catch (const TagOutdated &err)
      {
        #pragma omp critical
        error = err;
      }
#endif
    }
#ifdef _OPENMP
    if (error)
      throw error;
#endif

    /** gather the size of the copied data */
    uint64_t datasize(0);
    for (size_t i(0); i < tiles.size(); ++i)
      datasize += tiles[i].datasize;

    return datasize;
  }

  /** vector containing the tiles of the detector */
  vector<DetectorTile> tiles;

  /** the id that the detector should have within the pixeldetector part of
   *  the CASSEvent
   */
  int CASSID;

  /** how much the last tag should be advanced */
  int tagAdvance;

}; // end struct octal detector


/** a Machine value
 *
 * @author Lutz Foucar
 */
struct MachineValue
{
  /** constructor
   *
   * retrieve the hightag with the offline version of the API  using the
   * provided runnumber
   *
   * @param name The name of this Machine Value
   * @param runNbr The run number with which we can retrieve the high tag
   * @param blNbr The beamline number used to retrieve the right high tag number
   */
  MachineValue(const string &name, int runNbr, int blNbr)
    : cassname(name),
      name(name),
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
                                      const_cast<char*>(name.c_str()),
                                      _highTagNbr,tagList);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"MachineValue::copyData could not extract machine values of '" +
               name + "' ErrorCode is '" + toString(funcstatus) + "'");
      return 0;
    }
    if (machineValueStringList.size() != tagList.size())
    {
      Log::add(Log::ERROR,"MachineValue:copyData '" +
               name + "' did not return the right size");
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
      Log::add(Log::ERROR,"MachineValue::copyData '" + name + "' for tag '" +
               toString(tag) + "': String '" + machineValueStringList.back() +
               "' which is altered to '" + machineValueQString.toStdString() +
               "' to remove units, cannot be converted to double");
      return 0;
    }
    return sizeof(double);
  }

  /** the name of the machine value within the cassevent */
  string cassname;

  /** the name of the Machine value */
  string name;

private:
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
  : InputBase(ringbuffer,ratemeter,loadmeter,parent)
{
  Log::add(Log::VERBOSEINFO, "SACLAOnlineInput:: constructed");
}

void SACLAOnlineInput::runthis()
{
  /** load settings from the ini file */
  CASSSettings s;
  s.beginGroup("SACLAOnlineInput");

  /** load requested octal detectors */
  vector<OctalDetector> octalDetectors;
  int size = s.beginReadArray("OctalPixelDetectors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    int cassid(s.value("CASSID",-1).toInt());
    /** skip if the detector cass id  has not been set */
    if (cassid == -1)
      continue;
    /** load the infos for the detector */
    octalDetectors.push_back(OctalDetector());
    OctalDetector &det(octalDetectors.back());
    det.CASSID = cassid;
    det.tagAdvance = s.value("NextTagNumberAdvancedBy",2).toInt();
    Log::add(Log::INFO, "SACLAOnlineInput: Setting up octal detector with cassid '" +
             toString(cassid) +  "'.  The next tag number is guessed by " +
             "advancing the current one by '" + toString(det.tagAdvance) + "'");
    /** setup the individual tiles of the detector */
    int nTiles = s.beginReadArray("Tiles");
    for (int j(0); j<nTiles; ++j)
    {
      s.setArrayIndex(j);
      const string tilename(s.value("TileName","Invalid").toString().toStdString());
      octalDetectors.back().tiles.push_back(DetectorTile(tilename));
      DetectorTile &tile(octalDetectors.back().tiles.back());
      /** @note in online mode one gets the raw tile shape therefore one needs
       *        to remove the last 6 lines, which are used for calibration.
       *        Allow the user to choose how many rows need to be removed to
       *        prevent the necessity to recompile when the API changes with that
       *        respect.
       */
      tile.ysize -= s.value("NbrCalibrationRows",6).toUInt();
      tile.normalizeID = s.value("NormalizeTo",0).toInt()-1;
    }
    for (size_t j(0); j<det.tiles.size(); ++j)
    {
      DetectorTile &tile(det.tiles[j]);
      if (tile.normalizeID != -1)
        tile.relativeGain = tile.gain / det.tiles[tile.normalizeID].gain;
      Log::add(Log::INFO, "SACLAOnlineInput: Octal detector with cassid '"+
               toString(cassid) + "' has tile '" + tile.name +
               (tile.normalizeID == -1 ?"":" The tile will be normalized to tile '" +
                                        toString(tile.normalizeID + 1) + " (" +
                                        det.tiles[tile.normalizeID].name + ")'" +
                                        " with relative gain '" +
                                        toString(tile.relativeGain)) +
               "'. Tile Gain '" + toString(tile.gain) +
               "'; Tile shape '" + toString(tile.xsize) + "x" + toString(tile.ysize) +
               "'; Tile Gain '" + toString(tile.gain) + "'");
    }
    s.endArray();
  }
  s.endArray();

  /** quit if not at least one octal detector has been defined */
  if (octalDetectors.empty())
    throw invalid_argument("SACLAOnlineInput: Need to have at least one octal detector defined");


  /** load the beamline number of the beamline we're running on
   *  (needed to retieve database values)
   */
  int BeamlineNbr = s.value("BeamlineNumber",3).toInt();
  Log::add(Log::INFO, "SACLAOnlineInput: Using BeamlineNumber '" +
           toString(BeamlineNbr) + "'");


  /** load the requested database values */
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
    if (machineValName == "Invalid")
      continue;
    /** add the machine value */
    machineValues.push_back(MachineValue(machineValName,
                                         octalDetectors.back().runNumber(),
                                         BeamlineNbr));
    /** if the cass name is set then overwrite it withing the machinevalue */
    if (cassValName != "Invalid")
      machineValues.back().cassname = cassValName;
    Log::add(Log::INFO, "SACLAOnlineInput: Setting up Database value '" +
             machineValues.back().name + "' with CASSName '" +
             machineValues.back().cassname + "'");
  }
  s.endArray();

  s.endGroup();

  /** run until the thread is told to quit */
  Log::add(Log::DEBUG0,"SACLAOnlineInput::run(): starting loop");
  int lastTag(0);
  while(!shouldQuit())
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** retrieve a new element from the ringbuffer, continue with next iteration
     *  in case the retrieved element is the iterator to the last element of the
     *  buffer.
     */
    rbItem_t rbItem(getNextFillable());
    if (rbItem == _ringbuffer.end())
      continue;
    CASSEvent &evt(*rbItem->element);

    /** generate and set variable to keep the size of the retrieved data */
    uint64_t datasize(0);

    /** use try...catch to get notified when the requested tag data is not
     *  available anymore
     */
    try
    {
      /** get the part where the detector will be store in from the event */
      CASSEvent::devices_t &devices(evt.devices());
      CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
      if(devIt == devices.end())
        throw runtime_error("SACLAOnlineInput: CASSEvent does not contains a pixeldetector device");
      pixeldetector::Device &dev (*dynamic_cast<pixeldetector::Device*>(devIt->second));

      /** get the latest tag from the first defined octal detector */
      int latestTag(octalDetectors.front().latestTag(lastTag));

      /** only do something when the tag has advanced
       *  @note currently this not really necessary as the tag will always be
       *        advanced by the latestTag call. But this might be needed in
       *        future when it is possible to use the API function to retrieve
       *        the latest tag.
       */
      if (latestTag > lastTag)
      {
        /** set the event id */
        evt.id() = latestTag;

        /** copy octal detector data to cassevent and
         *  add the size in bytes copied to the total size retrieved
         */
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

        /** retrieve requested machinedata, copy it to the CASSEvent and add
         *  the size in bytes to the total size retrieved
         */
        vector<MachineValue>::iterator machIter(machineValues.begin());
        vector<MachineValue>::iterator machEnd(machineValues.end());
        for(; machIter != machEnd; ++machIter)
        {
          datasize += machIter->copyData(md,latestTag);
        }


        /** remember the latest tag */
        lastTag = latestTag;
      }
    }
    /** if the data for the tag wasn't available, reset the last tag and the datasize */
    catch (const TagOutdated &error)
    {
      Log::add(Log::ERROR,error.what());
      datasize = 0;
      lastTag = 0;
    }

    /** let the ratemeter know that we're done with this event with size
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
}


// Copyright (C) 2014, 2015 Lutz Foucar

/**
 * @file sacla_converter.cpp contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QRegExp>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "sacla_converter.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.hpp"
#include "cass.h"
#include "log.h"
#include "cass_exceptions.hpp"

using namespace cass;
using namespace std;


SACLAConverter::SACLAConverter()
{
#ifdef _OPENMP
  Log::add(Log::INFO, "SACLAConverter: Running with up to '" +
           toString(omp_get_max_threads()) + "' input threads");
#endif
}

SACLAConverter::detTileParams::~detTileParams()
{
  /** destroy the buffers */
//  string bout(name+ " destroying buffer " + toString(size_t(readBuf)));
//  cout << bout <<endl;
  st_destroy_stbuf(&readBuf);
//  string sout(name+ " destroying streamer " + toString(size_t(sreader)));
//  cout << sout <<endl;
  st_destroy_streader(&sreader);
}

void SACLAConverter::detTileParams::init(int runNbr, int blNbr)
{
  int funcstatus(0);
  /** create stream reader object */
  int r[] = {runNbr};
  funcstatus = st_create_streader(&sreader, name.c_str(), blNbr, 1, r);
  if (funcstatus)
    throw SaclaPixDetError(string("dettile::init: couldn't create stream ") +
                           "reader object for tile '" + name + "' on beamline '" +
                           toString(blNbr) + "' with run '" + toString(runNbr) +
                           "' ErrorCode is '" + toString(funcstatus) + "'");
  /** create the read buffer */
  funcstatus = st_create_stbuf(&readBuf, sreader);
  if (funcstatus)
    throw SaclaPixDetError(string("dettile::init: couldn't create stream ") +
                           "reader object for tile '" + name + "' on beamline '" +
                           toString(blNbr) + "' with run '" + toString(runNbr) +
                           "' ErrorCode is '" + toString(funcstatus) + "'");
}

void SACLAConverter::detTileParams::readFromStreamer(unsigned int tag)
{
  int funcstatus(0);
  /** collect the detector tile data */
  funcstatus = st_collect_data(readBuf,sreader,&tag);
  if (funcstatus)
    throw SaclaPixDetError(string("readFromStreamer: could not collect ") +
                           "data for '" + name + "' for tag '" + toString(tag) +
                           "' ErrorCode is '" + toString(funcstatus) + "'");
}

void SACLAConverter::detTileParams::cache()
{
  int funcstatus(0);
  /** the number of columns */
  funcstatus = st_read_det_xsize(&xsize, readBuf, 0);
  if (funcstatus)
    throw SaclaPixDetError(string("detTileParams::cache: error reading ") +
                           "xsize of '" + name + "' ErrorCode is '" +
                           toString(funcstatus) + "'");
  else
    Log::add(Log::INFO,"detTileParams::cache: Tile '" + name +
             "' has xsize '" + toString(xsize) + "'");

  /** the number of rows */
  funcstatus = st_read_det_ysize(&ysize, readBuf, 0);
  if (funcstatus)
    throw SaclaPixDetError(string("detTileParams::cache: error reading ") +
                           "ysize of '" + name + "' ErrorCode is '" +
                           toString(funcstatus) + "'");
  else
    Log::add(Log::INFO,"detTileParams::cache: Tile '" + name +
             "' has ysize '" + toString(ysize) + "'");

  /** the x-size of the pixels of the tile */
  funcstatus = mp_read_pixelsizex(&pixsizex_um, readBuf);
  if (funcstatus)
    throw SaclaPixDetError(string("detTileParams::cache: error reading ") +
                           "x pixelsize of '" + name + "' ErrorCode is '" +
                           toString(funcstatus) + "'");
  else
    Log::add(Log::INFO,"detTileParams::cache: Tile '" + name +
             "' has x pixelsize '" + toString(pixsizex_um) + "' um");
  /** the y-size of the pixels of the tile */
  funcstatus = mp_read_pixelsizey(&pixsizey_um,readBuf);
  if (funcstatus)
    throw SaclaPixDetError(string("detTileParams::cache: error reading ")+
                          "y pixelsize of '" + name + "' ErrorCode is '" +
                          toString(funcstatus) + "'");
  else
    Log::add(Log::INFO,"detTileParams::cache: Tile '" + name +
             "' has y-pixelsize '" + toString(pixsizey_um) + "' um");

  /** calc the total number of pixels form the x and y size */
  nPixels = xsize * ysize;
}

void SACLAConverter::detTileParams::copyTo(pixeldetector::Detector::frame_t::iterator pos)
{
  bytes_retrieved = 0.;
  int funcstatus(0);
  /** prepare the buffer where the data should be loaded to and retrieve the
   *  detector data from the reader buffer
   */
  vector<float> buffer(nPixels);
  funcstatus = st_read_det_data(&buffer.front(), readBuf, 0);
  if (funcstatus)
    throw SaclaPixDetError(string("detTileParams::copyTo: ") +
                           "could not retrieve data from buffer of '" + name +
                           "' ErrorCode is '" + toString(funcstatus) + "'");

  /** use transform to copy the data */
  transform(buffer.begin(), buffer.end(), pos,
            bind1st(multiplies<float>(),relativeGain));

  /** set the datasize of the retrieved data */
  bytes_retrieved = nPixels * sizeof(uint16_t);
}

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
    /** skip if the detector name has not been set */
    if (detID == "Invalid")
      continue;
    _octalDetectors.push_back(pixDets_t::value_type());
    _octalDetectors.back().CASSID = s.value("CASSID",0).toInt();
    _octalDetectors.back().normalize = s.value("NormalizeToAbsGain",true).toBool();
    _octalDetectors.back().tiles.resize(8);
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add ") +
             "octal detector with CASSID '" +
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
    _pixelDetectors.back().tiles.resize(1);
    _pixelDetectors.back().tiles[0].name = detID;
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add ") +
             "detector with CASSID '" +
             toString(_pixelDetectors.back().CASSID) + "'");
  }
  s.endArray();

  /** set the requested database values */
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
    Log::add(Log::INFO,string("SACLAConverter::loadSettings(): Add ") +
             "database value '" +
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
    /** create the sacla string buffer to retrieve the data */
    struct da_string_array *machineValueStringList = NULL;
    da_alloc_string_array(&machineValueStringList);
    /** retrive the data */
    funcstatus = sy_read_syncdatalist(machineValueStringList,
                                      mv.databaseName.c_str(),
                                      highTagNbr,
                                      tagList.size(),
                                      &(tagList.front()));
    if (funcstatus)
    {
      Log::add(Log::ERROR,string("SACLAConverter::cacheParameters could not ") +
               "cache values of '" + mv.databaseName + "' ErrorCode is '"
               + toString(funcstatus) + "'");
      da_destroy_string_array(&machineValueStringList);
      continue;
    }
    /** check if as many parameters as tags given have been returned. In case
     *  this number is different, something bad happened
     */
    int machineValueStringListSize = 0;
    da_getsize_string_array(&machineValueStringListSize, machineValueStringList);
    if (machineValueStringListSize != static_cast<int>(tagList.size()))
    {
      Log::add(Log::ERROR,"SACLAConverter:cacheParameters caching '" +
               mv.databaseName + "' did not return the right size");
      da_destroy_string_array(&machineValueStringList);
      continue;
    }
    /** convert the retrieved database values into double numbers
     *  and put them into the cache
     */
    vector<int>::const_iterator tag(tagList.begin());
    vector<int>::const_iterator tagListEnd(tagList.end());
    for (size_t i(0); tag != tagListEnd; ++tag, ++i)
    {
      /** check if retrieved value can be converted to double, and if so add it
       *  to the machine data, otherwise issue an error and add a 0 into the
       *  cache for that given tag.
       *  @note the retrieved values might contain the unit of the value in the
       *        string, therefore one has to remove all characters from the string
       */
      char * machineValueString(NULL);
      da_getstring_string_array(&machineValueString,
                                machineValueStringList,
                                i);
      QString machineValueQString(machineValueString);
      machineValueQString.remove(QRegExp("V|C|pulse|a\\.u\\."));
      bool isDouble(false);
      double machineValue(machineValueQString.toDouble(&isDouble));
      if (isDouble)
        mv.values[*tag] = machineValue;
      else
      {
        Log::add(Log::ERROR,"SACLAConverter::cacheParameters '" +
                 mv.databaseName + "' for tag '" + toString(*tag) +
                 "': String '" + machineValueString + "' which is altered to '" +
                 machineValueQString.toStdString() +
                 "' to remove units, cannot be converted to double. " +
                 "Setting it to 0");
        mv.values[*tag] = 0.;
      }
      free(machineValueString);
    }
    /** destroy the sacla string list */
    da_destroy_string_array(&machineValueStringList);
    /** output which database value has been cached */
    Log::add(Log::INFO,string("SACLAConverter::cacheParameters: ") +
             "cached values of database '" + mv.databaseName +
             "' into the CASS beamline value '" +
             mv.cassName + "'");
  }

  /** for all pixel dets, which consist of only 1 tile, retrieve the
   *  non-changing parameters from the first image
   */
  pixDets_t::iterator pixelDetsIter(_pixelDetectors.begin());
  pixDets_t::const_iterator pixelDetsEnd(_pixelDetectors.end());
  for (; pixelDetsIter != pixelDetsEnd; ++pixelDetsIter)
  {
    pixDets_t::value_type &pixdet(*pixelDetsIter);
    detTileParams &tile(pixdet.tiles.front());
    try
    {
      tile.init(runNbr,blNbr);
      tile.readFromStreamer(*first);
      tile.cache();
      pixdet.notLoaded = false;
    }//end try
    catch (const SaclaPixDetError &err)
    {
      Log::add(Log::ERROR,err.what());
      pixdet.notLoaded = true;
    }
  }//end pixdet loop


  /** for all octal dets retrieve the non changing parameters from the first
   *  image
   */
  pixDets_t::iterator octalDetsIter(_octalDetectors.begin());
  pixDets_t::const_iterator octalDetsEnd(_octalDetectors.end());
  for (; octalDetsIter != octalDetsEnd; ++octalDetsIter)
  {
    pixDets_t::value_type &octdet(*octalDetsIter);
    try
    {
      for (size_t i(0); i < octdet.tiles.size(); ++i)
      {
        /** get reference to the current tile */
        detTileParams &tile(octdet.tiles[i]);
        /** initialize the tiles streamer and buffer */
        tile.init(runNbr,blNbr);
        /** read data from the first tag into the buffer */
        tile.readFromStreamer(*first);
        /** cache the non-chaneging data */
        tile.cache();
        /** retrieve the additonal information of the tiles of an octal detector */
        int funcstatus(0);
        /** the position in x in the lab space in um */
        funcstatus = mp_read_posx(&(tile.posx_um), tile.readBuf);
        if (funcstatus)
          throw SaclaPixDetError(string("SACLAConverter::cacheParameters: ") +
                                 "pos X of '" + tile.name + "' ErrorCode is '" +
                                 toString(funcstatus) + "'");
        else
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' has pos x '" + toString(tile.posx_um) + "' um");

        /** the position in y in the lab space in um */
        funcstatus = mp_read_posx(&(tile.posy_um), tile.readBuf);
        if (funcstatus)
          throw SaclaPixDetError(string("SACLAConverter::cacheParameters: ") +
                                 "pos Y of '" + tile.name + "' ErrorCode is '" +
                                 toString(funcstatus) + "'");
        else
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' has pos y '" + toString(tile.posy_um) + "' um");

        /** the position in z in the lab space in um */
        funcstatus = mp_read_posx(&(tile.posz_um), tile.readBuf);
        if (funcstatus)
          throw SaclaPixDetError(string("SACLAConverter::cacheParameter: ") +
                                 "pos Z of '" + tile.name + "' ErrorCode is '" +
                                 toString(funcstatus) + "'");
        else
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' has pos z '" + toString(tile.posz_um) + "' um");

        /** the angle in degrees in the lab space */
        funcstatus = mp_read_rotationangle(&(tile.angle_deg), tile.readBuf);
        if (funcstatus)
          throw SaclaPixDetError(string("SACLAConverter::cacheParameter: ") +
                                 "angle of '" + tile.name + "' ErrorCode is '" +
                                 toString(funcstatus) + "'");
        else
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' has angle '" + toString(tile.angle_deg) +
                   "' degrees");

        /** the gain of the detector tile */
        funcstatus = mp_read_absgain(&(tile.gain), tile.readBuf);
        if (funcstatus)
          throw SaclaPixDetError(string("SACLAConverter::cacheParameter: ") +
                                 "absolute gain of '" + tile.name +
                                 "' ErrorCode is '" + toString(funcstatus) + "'");
        else
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' has absolute gain '" + toString(tile.gain) +
                   "'");
      }//end tile loop

      /** if the tiles of the octal detector should be normalized, calculate the
       *  relative gain of the individual tiles with respect to the first tile
       *  and store the relative gains within the tile
       */
      if (octdet.normalize)
      {
        detTileParams &firstTile(octdet.tiles.front());
        firstTile.normalize = false;
        for (size_t i(0); i < octdet.tiles.size(); ++i)
        {
          detTileParams &tile(octdet.tiles[i]);
          tile.normalize = true;
          tile.relativeGain = tile.gain / firstTile.gain;
          Log::add(Log::INFO,"SACLAConverter::cacheParameters: Tile '" +
                   tile.name + "' will be normalized with relative gain of '" +
                   toString(tile.relativeGain) + "'");
        }
      }

      /** get the total size of the detector */
      for (size_t i(0); i < octdet.tiles.size(); ++i)
      {
        detTileParams &tile(octdet.tiles[i]);
        octdet.nCols = tile.xsize;
        octdet.nRows +=  tile.ysize;
      }
      octdet.nPixels = octdet.nCols * octdet.nRows;
      Log::add(Log::INFO,string("SACLAConverter::cacheParameters: octal det ") +
               "has a shape of nCols '" + toString(octdet.nCols) + "', nRows '" +
               toString(octdet.nRows) + "', thus nPixels '" +
               toString(octdet.nPixels) + "'");
      octdet.notLoaded = false;
    }//end try
    catch (const SaclaPixDetError &err)
    {
      Log::add(Log::ERROR,err.what());
      octdet.notLoaded = true;
    }
  }//end octdet loop

}

uint64_t SACLAConverter::operator()(const int highTagNbr,
                                    const int tagNbr,
                                    CASSEvent& event)
{

  /** set the event id from the highTag and Tag number */
  CASSEvent::id_t eId(highTagNbr);
  eId = (eId << 32) + tagNbr;
  event.id() = eId;
  uint64_t datasize(0);

  /** get reference to the devices of the cassevent */
  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt;

  /** check if the event contains the machine data container, if so get a
   *  reference to it. Otherwise throw an error.
   */
  devIt = devices.find(CASSEvent::MachineData);
  if (devIt == devices.end())
    throw runtime_error(string("SACLAConverter():The CASSEvent does ") +
                        "not contain a Machine Data Device");
  MachineData::Device &md(dynamic_cast<MachineData::Device&>(*(devIt->second)));

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
  devIt = devices.find(CASSEvent::PixelDetectors);
  if(devIt == devices.end())
    throw runtime_error(string("SACLAConverter: CASSEvent does not ") +
                        "contain a pixeldetector device");
  pixeldetector::Device &dev(dynamic_cast<pixeldetector::Device&>(*(devIt->second)));

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
    md.BeamlineData()[tile.name + "_Width"]       = det.columns();
    md.BeamlineData()[tile.name + "_Height"]      = det.rows();
    md.BeamlineData()[tile.name + "_PixSizeX_um"] = tile.pixsizex_um;
    md.BeamlineData()[tile.name + "_PixSizeY_um"] = tile.pixsizey_um;

    /** retrieve the data with the right type */
    tile.readFromStreamer(tagNbr);
    tile.copyTo(det.frame().begin());

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
    det.columns() = octdet.nCols;
    det.rows()    = octdet.nRows;
    det.id()      = event.id();
    det.frame().resize(octdet.nPixels);

    /** copty the parameters of the tiles to the beamline data */
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
    {
      detTileParams &tile(octdet.tiles[i]);
      md.BeamlineData()[tile.name+"_Width"]      = tile.xsize;
      md.BeamlineData()[tile.name+"_Height"]     = tile.ysize;
      md.BeamlineData()[tile.name+"_PixSizeX_um"]= tile.pixsizex_um;
      md.BeamlineData()[tile.name+"_PixSizeY_um"]= tile.pixsizey_um;
      md.BeamlineData()[tile.name+"_PosX_um"]    = tile.posx_um;
      md.BeamlineData()[tile.name+"_PosY_um"]    = tile.posy_um;
      md.BeamlineData()[tile.name+"_PosZ_um"]    = tile.posz_um;
      md.BeamlineData()[tile.name+"_Angle_deg"]  = tile.angle_deg;
      md.BeamlineData()[tile.name+"_AbsGain"]    = tile.gain;
      md.BeamlineData()[tile.name+"_RelGain"]    = tile.relativeGain;
    }

    /** retrive the data of the tiles
     *  Using openmp to parallelize the retrieval of the detector data. The
     *  'pragma omp parallel for' statement says that the for loop should be
     *  parallelized. Within the loop all the declared variables are local to
     *  the thread and not shared. The 'num_threads' parameter allows to define
     *  how many threads should be used. Since the octal detector has 8 tiles,
     *  we only need 8 threads and not all that are available (which will be
     *  used if nothing is declared).
     */
#ifdef _OPENMP
    #pragma omp parallel for num_threads(octdet.tiles.size())
#endif
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
    {
      detTileParams &tile(octdet.tiles[i]);
      /** retrieve the data with the right type */
      tile.readFromStreamer(tagNbr);
      tile.copyTo(det.frame().begin() + i*tile.nPixels);
    }

    /** gather the size retrieved of all the tiles */
    for (size_t i = 0; i<octdet.tiles.size(); ++i)
      datasize += octdet.tiles[i].bytes_retrieved;
  }

  return datasize;
}

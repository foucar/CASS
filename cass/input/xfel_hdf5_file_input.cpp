// Copyright (C) 2017 Lutz Foucar

/**
 * @file xfel_hdf5_file_input.cpp contains class for reading xfel hdf5 data files
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <stdexcept>
#include <tr1/functional>

#include <QtCore/QFileInfo>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "xfel_hdf5_file_input.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"
#include "hdf5_handle.hpp"
#include "machine_device.hpp"
#include "acqiris_device.hpp"
#include "pixeldetector.hpp"

using namespace std;
using namespace std::tr1::placeholders;

using namespace cass;

namespace cass
{
/** a tile
 *
 * @author Lutz Foucar
 */
struct AGIPDTile
{
  /** define the type of the raw image */
  typedef vector<uint16_t> rawImage_t;

  /** define the type of the raw image */
  typedef vector<float> corImage_t;

  /** define the type of the mask data */
  typedef vector<uint8_t> mask_t;

  /** define the type of the gain data */
  typedef vector<uint8_t> gain_t;

  /** id of the tile */
  uint32_t id;

  /** define a pointer to the handler */
  typedef tr1::shared_ptr<hdf5::Handler> h5handle_t;

  /** handler to the file containing the tile data */
  h5handle_t fh;

  /** container for the corrrected image data */
  corImage_t dataCache;

  /** container for the mask data */
  mask_t maskCache;

  /** container for the gains settings data */
  gain_t gainCache;

  /** container for the train ids */
  vector<uint64_t> trainIds;

  /** map the uinque id to an index within the array */
  map<uint64_t, size_t> idToIdx;

  /** name of the data dataset in the hdf5 file */
  std::string dataDsetName;

  /** name of the mask dataset in the hdf5 file */
  std::string maskDsetName;

  /** name of the mask dataset in the hdf5 file */
  std::string gainDsetName;

  /** name of the train id dataset in the hdf5 file */
  std::string trainIdDsetName;

  /** name of the pulse id dataset in the hdf5 file */
  std::string pulseIdDsetName;

  /** name of the cell id dataset in the hdf5 file */
  std::string cellIdDsetName;

  /** the datasize of the data tile */
  size_t size;

  /** the number of columns of the tile */
  size_t nCols;

  /** the number of rows of the tile */
  size_t nRows;
};

/** a struct combining the information needed to get machine values
 *
 * @author Lutz Foucar
 */
struct MachineInfo
{
  /** define a pointer to the handler */
  typedef tr1::shared_ptr<hdf5::Handler> h5handle_t;

  /** handler to the file containing the tile data */
  h5handle_t fh;

  /** name of the machine dataset in the hdf5 file */
  std::string hdf5DsetName;

  /** name of the machineData within the CASSEvent */
  std::string cassname;

  /** name of the train id dataset in the hdf5 file */
  std::string trainIdDsetName;

  /** map the uinque id to an index within the array */
  map<uint64_t, size_t> idxToTrainId;

  /** cache to store the data in */
  vector<float> cache;

  /** shape of the cached data */
  hdf5::shape_t shape;
};

/** get the tile from the hdf5 file and copy it to the correct position in the
 *  frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void tileToFrames(const AGIPDTile& tile,
                  const uint64_t &eventId,
                  pixeldetector::Detector::frame_t::iterator dframe,
                  pixeldetector::Detector::frame_t::iterator gframe,
                  pixeldetector::Detector::frame_t::iterator mframe,
                  const bool precache)
{
  /** return if the tile data for the eventid id is not available */
  map<uint64_t,size_t>::const_iterator idxIt(tile.idToIdx.find(eventId));
  if (idxIt == tile.idToIdx.end())
    return;

  if (precache)
  {
    /** @NOTE instead of reading the partial data from file, copy from the cached data
     * to see if thats faster.
     */
    /** get iterators to start and end of requested image from the cache */
    auto dTileBegin(tile.dataCache.begin());
    advance(dTileBegin,tile.size*idxIt->second);
    auto dTileEnd(dTileBegin);
    advance(dTileEnd,tile.size);
    /** now read the current images tile into the frame */
    auto dTileInFrame(dframe+tile.id*tile.size);
    copy(dTileBegin,dTileEnd,dTileInFrame);

    /** get iterators to start and end of requested image from the cache */
    auto gTileBegin(tile.gainCache.begin());
    advance(gTileBegin,tile.size*idxIt->second);
    auto gTileEnd(gTileBegin);
    advance(gTileEnd,tile.size);
    /** now read the current images tile into the frame */
    auto gTileInFrame(gframe+tile.id*tile.size);
    copy(gTileBegin,gTileEnd,gTileInFrame);

    /** get iterators to start and end of requested image from the cache */
    auto mTileBegin(tile.maskCache.begin());
    advance(mTileBegin,tile.size*idxIt->second);
    auto mTileEnd(mTileBegin);
    advance(mTileEnd,tile.size);
    /** now read the current images tile into the frame */
    auto mTileInFrame(mframe+tile.id*tile.size);
    copy(mTileBegin,mTileEnd,mTileInFrame);
  }
  else
  {
    /** create the partiality parameters for retrieving only the current images'
     *  tile from the dataset
     */
    hdf5::partiality_t part;
    part.dims.push_back(1);
    part.dims.push_back(tile.nRows);
    part.dims.push_back(tile.nCols);

    part.offset.resize(part.dims.size(),0);
    part.offset[0] = idxIt->second;

    part.count.assign(part.dims.begin(),part.dims.end());
    part.block.resize(part.dims.size(),1);
    part.stride.resize(part.dims.size(),1);

    /** get iterator to the start of the tile within the frame */
    pixeldetector::Detector::frame_t::iterator dTileInFrame(dframe+tile.id*tile.size);
    pixeldetector::Detector::frame_t::iterator gTileInFrame(gframe+tile.id*tile.size);
    pixeldetector::Detector::frame_t::iterator mTileInFrame(mframe+tile.id*tile.size);

    /** now read the current images tile into the frame */
    tile.fh->readPartialMultiDim<float>(dTileInFrame,part,tile.dataDsetName);
    tile.fh->readPartialMultiDim<float>(gTileInFrame,part,tile.gainDsetName);
    tile.fh->readPartialMultiDim<float>(mTileInFrame,part,tile.maskDsetName);
  }
}

}//end namespace cass

void XFELHDF5FileInput::instance(string filelistname,
                             RingBuffer<CASSEvent> &ringbuffer,
                             Ratemeter &ratemeter, Ratemeter &loadmeter,
                             bool quitWhenDone,
                             QObject *parent)
{
  if(_instance)
    throw logic_error("HDF5FileInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new XFELHDF5FileInput(filelistname,ringbuffer,ratemeter,loadmeter,quitWhenDone,parent));
}

XFELHDF5FileInput::XFELHDF5FileInput(string filelistname,
                                     RingBuffer<CASSEvent> &ringbuffer,
                                     Ratemeter &ratemeter, Ratemeter &loadmeter,
                                     bool quitWhenDone,
                                     QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname),
    _counter(0),
    _scounter(0),
    _totalevents(0)
{
  Log::add(Log::VERBOSEINFO, "HDF5FileInput::HDF5FileInput: constructed");
}

void XFELHDF5FileInput::load()
{
}

void XFELHDF5FileInput::runthis()
{
  /** load the settings from the ini file */
  CASSSettings s;
  s.beginGroup("XFELHDF5FileInput");
  const bool precache(s.value("PreCacheData",true).toBool());
  s.beginGroup("AGIPD");

  Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::runthis: setup the parameters from the ini file");

  /** get the cassids for the individual agipd images */
  int DataCASSID(s.value("DataCASSID",30).toInt());
  int MaskCASSID(s.value("MaskCASSID",31).toInt());
  int GainCASSID(s.value("GainCASSID",32).toInt());

  /** get info for the individual tiles */
  vector<AGIPDTile> agiptiles;
  int size = s.beginReadArray("Tiles");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    /** create a new tile */
    AGIPDTile tile;
    const int tileid(s.value("Id",-1).toInt());
    if (tileid == -1)
      continue;
    //cout << tileid <<endl;
    tile.id = tileid;
    const string fn(s.value("HDF5FileName","Invalid").toString().toStdString());
    if (fn == "Invalid")
      continue;
    //cout << fn <<endl;
    try
    {
      tile.fh = AGIPDTile::h5handle_t(new hdf5::Handler());
      tile.fh->open(fn,"r");
    }
    catch (const std::invalid_argument & error)
    {
      Log::add(Log::ERROR,string(error.what()) + ": File '"+fn+"' doesn't exist; ignoring it");
      continue;
    }
    tile.dataDsetName = s.value("DataHDF5Key","Invalid").toString().toStdString();
    if (tile.dataDsetName == "Invalid")
      continue;
    //cout << tile.dataDsetName <<endl;
    tile.maskDsetName = s.value("MaskHDF5Key","Invalid").toString().toStdString();
    if (tile.maskDsetName == "Invalid")
      continue;
    //cout << tile.maskDsetName <<endl;
    tile.gainDsetName = s.value("GainHDF5Key","Invalid").toString().toStdString();
    if (tile.gainDsetName == "Invalid")
      continue;
    //cout << tile.gainDsetName <<endl;
    tile.trainIdDsetName = s.value("TrainIdHDF5Key","Invalid").toString().toStdString();
    if (tile.trainIdDsetName == "Invalid")
      continue;
    //cout << tile.trainIdDsetName <<endl;
    tile.pulseIdDsetName = s.value("PulseIdHDF5Key","Invalid").toString().toStdString();
    if (tile.pulseIdDsetName == "Invalid")
      continue;
    //cout << tile.pulseIdDsetName <<endl;
    tile.cellIdDsetName = s.value("CellIdHDF5Key","Invalid").toString().toStdString();
    if (tile.cellIdDsetName == "Invalid")
      continue;
    //cout << tile.cellIdDsetName <<endl;
    tile.nRows = 512;
    tile.nCols = 128;
    tile.size = tile.nRows * tile.nCols;
    Log::add(Log::INFO,"XFELHDF5FileInput: adding agipd tile '" + toString(tile.id) +
                       "' to list of tiles to be processed");
    agiptiles.push_back(tile);
  }
//  if (agiptiles.size() != 16)
//    throw invalid_argument("Wrong size of agipd tiles '" + toString(agiptiles.size()) + "'");
  s.endArray(); // Tiles
  s.endGroup(); // AGPID

  /** get the user defined machine values */
  vector<MachineInfo> machinevalues;
  size = s.beginReadArray("MachineData");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    /** create a new machineInfo */
    MachineInfo mi;

    /** the filename of the file containing the machine value */
    const string fn(s.value("HDF5FileName","Invalid").toString().toStdString());
    if (fn == "Invalid")
      continue;
    //cout << fn <<endl;
    try
    {
      mi.fh = MachineInfo::h5handle_t(new hdf5::Handler());
      mi.fh->open(fn,"r");
    }
    catch (const std::invalid_argument & error)
    {
      Log::add(Log::ERROR,string(error.what()) + ": File '"+fn+"' doesn't exist; ignoring it");
      continue;
    }

    /** the name of the machine data within the cassevent */
    mi.cassname = s.value("CASSName","Invalid").toString().toStdString();
    if (mi.cassname == "Invalid")
      continue;
    //cout << mi.cassname << endl;

    /** the name of the machine data within the cassevent */
    mi.hdf5DsetName = s.value("MachineHDF5Key","Invalid").toString().toStdString();
    if (mi.hdf5DsetName == "Invalid")
      continue;
    //cout << mi.hdf5DsetName << endl;

    mi.trainIdDsetName = s.value("TrainIdHDF5Key","Invalid").toString().toStdString();
    if (mi.trainIdDsetName == "Invalid")
      continue;
    //cout << tile.trainIdDsetName <<endl;

    machinevalues.push_back(mi);
  }
  s.endArray(); // MachineData

  s.endGroup(); // XFEHDF5FileInput

  _status = lmf::PausableThread::running;

  /** for all the tiles make a map of ids generated from the trainId and the pulseId with the
   *  corresponding index to the data
   *  Using openmp to parallelize the retrieval of the detector data. The
   *  'pragma omp parallel for' statement says that the for loop should be
   *  parallelized. Within the loop all the declared variables are local to
   *  the thread and not shared. The 'num_threads' parameter allows to define
   *  how many threads should be used. Since the agip detector has 16 tiles,
   *  we only need 16 threads and not all that are available (which will be
   *  used if nothing is declared).
   */
#ifdef _OPENMP
    #pragma omp parallel for num_threads(agiptiles.size())
#endif
//  for (auto &tile : agiptiles)
  for (size_t i = 0; i<agiptiles.size(); ++i)
  {
    auto &tile(agiptiles[i]);
    Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::runthis: create eventid map for agipd tile '"+
                               toString(tile.id) + "' and precache the data");
    hdf5::shape_t shape;
    tile.fh->readMultiDim<uint64_t>(tile.trainIds,shape,tile.trainIdDsetName);
    vector<uint64_t> pulseIds;
    tile.fh->readMultiDim<uint64_t>(pulseIds,shape,tile.cellIdDsetName);
    //tile.fh->readMultiDim<uint64_t>(pulseIds,shape,tile.pulseIdDsetName);
    if (pulseIds.size() != tile.trainIds.size())
      cout << "BAD!!"<<endl;
    for (size_t i(0); i<pulseIds.size(); ++i)
    {
      const uint64_t trainId(tile.trainIds[i]);
      const uint64_t pulseId(pulseIds[i]);
      const uint64_t id(((trainId & 0x0000FFFFFFFFFFFF)<<16) +
                         (pulseId & 0x000000000000FFFF));
      tile.idToIdx[id] = i;
    }

    /**  preload the data to check if it improves the speed of the analysis */
    if (precache)
    {
      tile.fh->readMultiDim<AGIPDTile::corImage_t::value_type>(tile.dataCache,shape,tile.dataDsetName);
      tile.fh->readMultiDim<AGIPDTile::mask_t::value_type>(tile.maskCache,shape,tile.maskDsetName);
      tile.fh->readMultiDim<AGIPDTile::gain_t::value_type>(tile.gainCache,shape,tile.gainDsetName);
    }
    Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::runthis: done setting up agipd tile '"+
                               toString(tile.id) + "'");
  }

  /** create a map that tells the idx for each trainids of the machine values */
  for (auto &mv : machinevalues)
  {
    Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::runthis: create trainId map for machinvalue '"+
                               mv.cassname + "' and precache the data");
    hdf5::shape_t shape;
    vector<uint64_t> trainIds;
    mv.fh->readMultiDim<uint64_t>(trainIds,shape,mv.trainIdDsetName);
    for (size_t i(0); i<trainIds.size(); ++i)
      mv.idxToTrainId[trainIds[i]] = i;

    /** pre cache the data to see whether its faster to analyze */
    if (precache)
    {
      mv.fh->readMultiDim<float>(mv.cache,mv.shape,mv.hdf5DsetName);
    }
  }

  /** generate container list of all the unique trainIds available */
  std::set<uint64_t> trainIds;
  for (const auto &tile : agiptiles)
  {
    for (const auto &trainId : tile.trainIds)
    {
      trainIds.emplace(trainId);
    }
  }

  Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::runthis: the list of trainIds is '"+
                             toString(trainIds.size()) + "' long");

  _totalevents = trainIds.size() * 60;

  /** now go through all the trains */
  for (const auto &trainId : trainIds)
//  auto iTrainId(trainIds.begin());
//  advance(iTrainId,250);
//  for (; iTrainId != trainIds.end(); ++iTrainId)
  {
//    auto trainId(*iTrainId);
    /** add break point here */
    if (shouldQuit()) break;

    /** go through all the nominal available pulses within a train */
    for (size_t pulseId(0); pulseId<60; ++pulseId)
    {
      /** add break point here */
      if (shouldQuit()) break;

      /** retrieve a new element from the ringbuffer. If one didn't get a
       *  an element (when the end iterator of the buffer is returned).
       *  Continue to the next iteration, where it is checked if the thread
       *  should quit.
       */
      bool isGood(true);
      rbItem_t rbItem(getNextFillable());
      if (rbItem == _ringbuffer.end())
        continue;
      CASSEvent& evt(*rbItem->element);

      /** generate an unique eventid from the pulse id and the trainId */
      evt.id() = (((trainId & 0x0000FFFFFFFFFFFF)<<16) +
                   (pulseId & 0x000000000000FFFF));

      /** get reference to all devices of the CASSEvent and an iterator*/
      CASSEvent::devices_t &devices(evt.devices());
      CASSEvent::devices_t::iterator devIt;
      /** retrieve the pixel detector part of the cassevent */
      devIt = devices.find(CASSEvent::PixelDetectors);
      if(devIt == devices.end())
        throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
                            " contain a pixeldetector device");
      pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));

      /** retrieve the right detectors from the cassevent and reset it*/
      const size_t nRows(512);
      const size_t nCols(128);
      const size_t nTiles(16);
      /** the data frame */
      pixeldetector::Detector &data(pixdev.dets()[DataCASSID]);
      data.id() = evt.id();
      data.columns() = nCols;
      data.rows() = nRows*nTiles;
      pixeldetector::Detector::frame_t &dataframe(data.frame());
      dataframe.clear();
      dataframe.resize(nRows*nCols*nTiles,0);
      /** the mask frame */
      pixeldetector::Detector &mask(pixdev.dets()[MaskCASSID]);
      mask.id() = evt.id();
      mask.columns() = nCols;
      mask.rows() = nRows*nTiles;
      pixeldetector::Detector::frame_t &maskframe(mask.frame());
      maskframe.clear();
      maskframe.resize(nRows*nCols*nTiles,0);
      /** the gain frame */
      pixeldetector::Detector &gain(pixdev.dets()[GainCASSID]);
      gain.id() = evt.id();
      gain.columns() = nCols;
      gain.rows() = nRows*nTiles;
      pixeldetector::Detector::frame_t &gainframe(gain.frame());
      gainframe.clear();
      gainframe.resize(nRows*nCols*nTiles,0);

      /** find the tiles that correspond to the the made up eventid and copy them to the right pos
       *  within the detector image
       */
      for_each(agiptiles.begin(),agiptiles.end(),
               tr1::bind(tileToFrames, _1,
                                       evt.id(),
                                       dataframe.begin(),
                                       gainframe.begin(),
                                       maskframe.begin(),
                                       precache));
      /** ensure that all values in the frame are numbers */
      for (auto &pixel : dataframe) if (!isfinite(pixel)) pixel = 0;


      /** retrieve the machine detector part of the cassevent */
      devIt = devices.find(CASSEvent::MachineData);
      if(devIt == devices.end())
        throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
                                   " contain a machinedata device");
      MachineData::Device &md (dynamic_cast<MachineData::Device&>(*(devIt->second)));
      md.BeamlineData()["PulseId"] = pulseId;
      md.BeamlineData()["TrainId"] = trainId;
      /** find the additional data for the machine data */
      for (const auto& mv : machinevalues)
      {
        /** skip if the tile data for the eventid id is not available */
        map<uint64_t,size_t>::const_iterator idxIt(mv.idxToTrainId.find(trainId));
        if (idxIt == mv.idxToTrainId.end())
          continue;
        if (precache)
        {
          auto mBegin(mv.cache.begin());
          advance(mBegin, pulseId + idxIt->second*mv.shape[1]);
          md.BeamlineData()[mv.cassname] = *mBegin;
        }
        else
        {
          hdf5::partiality_t part;
          part.dims.push_back(1);
          part.dims.push_back(1);

          part.offset.resize(part.dims.size(),0);
          part.offset[0] = idxIt->second;
          part.offset[1] = pulseId;

          part.count.assign(part.dims.begin(),part.dims.end());
          part.block.resize(part.dims.size(),1);
          part.stride.resize(part.dims.size(),1);

          vector<float> mval(1,0);
          mv.fh->readPartialMultiDim<float>(mval.begin(), part, mv.hdf5DsetName);

          md.BeamlineData()[mv.cassname] = mval.front();
        }
      }

      /** tell the ringbuffer that we're done with the event */
      newEventAdded(0);
      ++_counter;
      _ringbuffer.doneFilling(rbItem, isGood);
    }
  }


//  /** now go through all set of files */
//  for (size_t i(0); (!shouldQuit()) && (i<filesetlist.size());++i)
//  {
//    fileset_t& fileset(filesetlist[i]);
//
//    /** get the shape from the first datafile (should be the same for all data
//     *  files). The first dimension is the number of images, so we iterate over
//     *  this number
//     */
//    const hdf5::shape_t shape(fileset[0].fh->shape(fileset[0].dataDsetName));
//    const size_t nCols(shape[shape.size()-1]);
//    const size_t nRows(shape[shape.size()-2]);
//    const size_t nTiles(fileset.size());
//    _totalevents = shape[0];
//    fileset_t::iterator fsit(fileset.begin());
//    while(fsit < fileset.end())
//    {
//      fsit->nCols = nCols;
//      fsit->nRows = nRows;
//      fsit->size = (nCols*nRows);
//      ++fsit;
//    }
//    /** pre cache the data */
//    for_each(fileset.begin(),fileset.end(),preCacheData);
//
//    /** get all the trainids and the pulseids from the first datafile, they
//     *  later used to compile a unique eventid
//     */
//    vector<uint64_t> trainIds;
//    size_t length;
//    fileset[0].fh->readArray(trainIds,length,trainIdKey);
//
//    vector<uint64_t> pulseIds;
//    fileset[0].fh->readArray(pulseIds,length,pulseIdKey);
//
//    vector<uint16_t> cellIds;
//    fileset[0].fh->readArray(cellIds,length,cellIdKey);
//
//    for (size_t iTrain(0); (!shouldQuit()) && (iTrain < nTrains);++iTrain)
//    {
//      for (size_t iImage(0); (!shouldQuit()) && (iImage < nImages);++iImage)
//      {
//        /** calculate the which Event in the file should be retrieved */
//        size_t iImageInFile(iTrain*nImagesInTrainTotal + iImage*imageStride + imageOffset);
//        /** flag that tell whether the data is good */
//        bool isGood(true);
//        /** retrieve a new element from the ringbuffer. If one didn't get a
//       *  an element (when the end iterator of the buffer is returned).
//       *  Continue to the next iteration, where it is checked if the thread
//       *  should quit.
//       */
//        rbItem_t rbItem(getNextFillable());
//        if (rbItem == _ringbuffer.end())
//          continue;
//        CASSEvent& evt(*rbItem->element);
//
//        /** create the event id from the trainid and the pulseid */
//        evt.id() = ((trainIds[iImageInFile] & 0x0000FFFFFFFFFFFF)<< 16) +
//            ((pulseIds[iImageInFile] & 0x000000000000FFFF));
//
//        /** get reference to all devices of the CASSEvent and an iterator*/
//        CASSEvent::devices_t &devices(evt.devices());
//        CASSEvent::devices_t::iterator devIt;
//        /** retrieve the pixel detector part of the cassevent */
//        devIt = devices.find(CASSEvent::PixelDetectors);
//        if(devIt == devices.end())
//          throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
//                              " contain a pixeldetector device");
//        pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
//        /** retrieve the right detector from the cassevent and reset it*/
//        pixeldetector::Detector &data(pixdev.dets()[DataCASSID]);
//        pixeldetector::Detector::frame_t &dataframe(data.frame());
//        dataframe.clear();
//        dataframe.resize(nRows*nCols*nTiles);
//        /** copy the det data to the frame */
//        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyDataTileToFrame,_1,
//        //                                                       dataframe.begin(),iImageInFile));
//        for_each(fileset.begin(),fileset.end(),tr1::bind(copyCorImageFromCacheToFrame,_1,
//                                                         dataframe.begin(),iImageInFile));
//        /** set the detector parameters and add the event id */
//        data.columns() = nCols;
//        data.rows() = nRows*nTiles;
//        data.id() = evt.id();
//        /** retrieve the right detector from the cassevent and reset it*/
//        pixeldetector::Detector &gain(pixdev.dets()[GainCASSID]);
//        pixeldetector::Detector::frame_t &gainframe(gain.frame());
//        gainframe.clear();
//        gainframe.resize(nRows*nCols*nTiles);
//        /** copy the det data to the frame */
//        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyGainTileToFrame,_1,
//        //                                                       gainframe.begin(),iImageInFile));
//        for_each(fileset.begin(),fileset.end(),tr1::bind(copyGainFromCacheToFrame,_1,
//                                                         gainframe.begin(),iImageInFile));
//        /** set the detector parameters and add the event id */
//        gain.columns() = nCols;
//        gain.rows() = nRows*nTiles;
//        gain.id() = evt.id();
//        /** retrieve the right detector to hold the mask from the cassevent and reset it*/
//        pixeldetector::Detector &mask(pixdev.dets()[MaskCASSID]);
//        pixeldetector::Detector::frame_t &maskframe(mask.frame());
//        maskframe.clear();
//        maskframe.resize(nRows*nCols*nTiles);
//        /** copy the det data to the frame */
//        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyTileToMask,_1,
//        //                                                       maskframe.begin(),iImageInFile));
//        for_each(fileset.begin(),fileset.end(),tr1::bind(copyMaskFromCacheToFrame,_1,
//                                                         maskframe.begin(),iImageInFile));
//        /** set the detector parameters and add the event id */
//        mask.columns() = nCols;
//        mask.rows() = nRows*nTiles;
//        mask.id() = evt.id();
//
//        /** retrieve the machine detector part of the cassevent */
//        devIt = devices.find(CASSEvent::MachineData);
//        if(devIt == devices.end())
//          throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
//                              " contain a machinedata device");
//        MachineData::Device &md (dynamic_cast<MachineData::Device&>(*(devIt->second)));
//        md.BeamlineData()["CellId"] = cellIds[iImageInFile];
//        md.BeamlineData()["PulseId"] = pulseIds[iImageInFile];
//        md.BeamlineData()["TrainId"] = trainIds[iImageInFile];
//
//        /** tell the ringbuffer that we're done with the event */
//        newEventAdded(0);
//        ++_counter;
//        _ringbuffer.doneFilling(rbItem, isGood);
//      }
//    }
//  }

  /** quit the program if requested otherwise wait until the program is signalled
   *  to quit
   */
  Log::add(Log::INFO,"XFELHDF5FileInput::run(): Finished with all files.");
  if(!_quitWhenDone)
    while(!shouldQuit())
      this->sleep(1);
  Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::run(): closing the input");
  Log::add(Log::INFO,"XFELHDF5FileInput::run(): Extracted '" +
           toString(eventcounter()) + "' events.");
}


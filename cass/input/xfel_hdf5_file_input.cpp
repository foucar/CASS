// Copyright (C) 2017 Lutz Foucar

/**
 * @file xfel_hdf5_file_input.cpp contains class for reading xfel hdf5 data files
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <tr1/functional>

#include <QtCore/QFileInfo>

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
  corImage_t corImage;

  /** container for the raw image data */
  rawImage_t rawImage;

  /** container for the mask data */
  mask_t mask;

  /** container for the gains settings data */
  gain_t gain;

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

  /** the datasize of the data tile */
  size_t size;

  /** the number of columns of the tile */
  size_t nCols;

  /** the number of rows of the tile */
  size_t nRows;
};

/** pre-cache all the data by reading it into the corresponding memory locations
 *
 * @author Lutz Foucar
 *
 * @param tile reference to tile that should be precached
 */
void preCacheData(AGIPDTile& tile)
{
  /** now read the current images tile into the frame */
  hdf5::shape_t shape;
//  string output = "read data from file " +tile.fh->filename()+ "\n";
//  cout << output;
  tile.fh->readMultiDim<AGIPDTile::corImage_t::value_type>(tile.corImage,shape,tile.dataDsetName);
  tile.fh->readMultiDim<AGIPDTile::mask_t::value_type>(tile.mask,shape,tile.maskDsetName);
  tile.fh->readMultiDim<AGIPDTile::gain_t::value_type>(tile.gain,shape,tile.gainDsetName);
//  output = "done with file " +tile.fh->filename()+ "\n";
//  cout << output;
}

/** get the tile from the hdf5 file and copy it to the correct position in the
 *  frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyDataTileToFrame(const AGIPDTile& tile,
                         pixeldetector::Detector::frame_t::iterator frame,
                         int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);
  /** create the partiality parameters for retrieving only the current images'
   *  tile from the dataset
   */
  hdf5::partiality_t part;
  part.dims.push_back(1);
  //part.dims.push_back(1);
  part.dims.push_back(tile.nRows);
  part.dims.push_back(tile.nCols);

  part.offset.resize(part.dims.size(),0);
  part.offset[0] = imageNbr;
  //part.offset[1] = 1;

  part.count.assign(part.dims.begin(),part.dims.end());
  part.block.resize(part.dims.size(),1);
  part.stride.resize(part.dims.size(),1);

  /** now read the current images tile into the frame */
  tile.fh->readPartialMultiDim<float>(tileInFrame,part,tile.dataDsetName);
}

/** get the tile from the cached data and copy it to the correct position in the
 *  frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyCorImageFromCacheToFrame(const AGIPDTile& tile,
                                  pixeldetector::Detector::frame_t::iterator frame,
                                  int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);
  /** get iterators to start and end of requested image from the cache */
  AGIPDTile::corImage_t::const_iterator tileBegin(tile.corImage.begin());
  advance(tileBegin,tile.size*imageNbr);
  AGIPDTile::corImage_t::const_iterator tileEnd(tileBegin);
  advance(tileEnd,tile.size);
  /** now read the current images tile into the frame */
  copy(tileBegin,tileEnd,tileInFrame);
}


/** get the tile from the hdf5 file and copy it to the correct position in the
 *  frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyGainTileToFrame(const AGIPDTile& tile,
                         pixeldetector::Detector::frame_t::iterator frame,
                         int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);
  /** create the partiality parameters for retrieving only the current images'
   *  tile from the dataset
   */
  hdf5::partiality_t part;
  part.dims.push_back(1);
  part.dims.push_back(tile.nRows);
  part.dims.push_back(tile.nCols);

  part.offset.resize(part.dims.size(),0);
  part.offset[0] = imageNbr;

  part.count.assign(part.dims.begin(),part.dims.end());
  part.block.resize(part.dims.size(),1);
  part.stride.resize(part.dims.size(),1);

  /** now read the current images tile into the frame */
  tile.fh->readPartialMultiDim<float>(tileInFrame,part,tile.gainDsetName);
}


/** get the gain from the cached data and copy it to the correct position in the
 *  frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyGainFromCacheToFrame(const AGIPDTile& tile,
                              pixeldetector::Detector::frame_t::iterator frame,
                              int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);
  /** get iterators to start and end of requested image from the cache */
  AGIPDTile::gain_t::const_iterator tileBegin(tile.gain.begin());
  advance(tileBegin,tile.size*imageNbr);
  AGIPDTile::gain_t::const_iterator tileEnd(tileBegin);
  advance(tileEnd,tile.size);
  /** now read the current images tile into the frame */
  copy(tileBegin,tileEnd,tileInFrame);
}

/** get the mask tile from the hdf5 file and copy it to the correct
 *  position in the frame
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyTileToMask(const AGIPDTile& tile,
                    pixeldetector::Detector::frame_t::iterator frame,
                    int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);
  /** create the partiality parameters for retrieving only the current images'
   *  tile from the dataset
   */
  hdf5::partiality_t part;
  part.dims.push_back(1);
  part.dims.push_back(tile.nRows);
  part.dims.push_back(tile.nCols);
  part.dims.push_back(3);

  part.offset.resize(part.dims.size(),0);
  part.offset[0] = imageNbr;

  part.count.assign(part.dims.begin(),part.dims.end());
  part.block.resize(part.dims.size(),1);
  part.stride.resize(part.dims.size(),1);

  /** read the data into a temporary space, since its 3 times bigger than the
   *  detector image, due to the fact that the masks for all 3 gain stages are
   *  in separate dimensions
   */
  vector<uint8_t> mask(tile.size*3,0);
  tile.fh->readPartialMultiDim<uint8_t>(mask.begin(),part,tile.maskDsetName);
  vector<uint8_t>::const_iterator maskIt(mask.begin());

  /** read the gain into the memory */
  vector<uint8_t> gain(tile.size,4);
  part.dims.erase(part.dims.end()-1);
  part.offset.erase(part.offset.end()-1);
  part.count.erase(part.count.end()-1);
  part.block.erase(part.block.end()-1);
  part.stride.erase(part.stride.end()-1);
  tile.fh->readPartialMultiDim<uint8_t>(gain.begin(),part,tile.gainDsetName);
  vector<uint8_t>::const_iterator gainIt(gain.begin());
  /** now read the current mask tile into the frame, load the correct mask value
   *  for the gain that the pixel is in
   */
  for (size_t i(0); i<(tile.size); ++i)
  {
    *tileInFrame += maskIt[*gainIt];
    advance(maskIt,3);
    advance(gainIt,1);
    advance(tileInFrame,1);
  }
}

/** get the mask from the cached data and copy it to the correct position in the
 *  frame but only that mask from the gain that the pixel is in
 *
 * @author Lutz Foucar
 *
 * @param tile reference to the tile to be copied
 * @param frame iterator to the frame where the tile data should be written to.
 * @param imageNbr which image within the file should be retrieved
 */
void copyMaskFromCacheToFrame(const AGIPDTile& tile,
                              pixeldetector::Detector::frame_t::iterator frame,
                              int imageNbr)
{
  /** get iterator to the start of the tile within the frame */
  pixeldetector::Detector::frame_t::iterator tileInFrame(frame+tile.id*tile.size);

  /** get iterators to start of requested image from the cache */
  AGIPDTile::gain_t::const_iterator gainIt(tile.gain.begin());
  advance(gainIt,tile.size*imageNbr);
  /** get iterator to the start of the masks */
  AGIPDTile::mask_t::const_iterator maskIt(tile.mask.begin());
  advance(maskIt,tile.size*imageNbr*3);
  /** now read the current mask tile into the frame, load the correct mask value
   *  for the gain that the pixel is in
   */
  for (size_t i(0); i<(tile.size); ++i)
  {
    *tileInFrame += maskIt[*gainIt];
    advance(maskIt,3);
    advance(gainIt,1);
    advance(tileInFrame,1);
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
  s.beginGroup("AGIPD");

  /** list of datasetnames of the individual tiles */
  typedef map<int,string> machineVals_t;
  machineVals_t datatilenames;
  int DataCASSID(s.value("DataCASSID",30).toInt());
  int size = s.beginReadArray("DataTiles");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    datatilenames[i] = s.value("HDF5Key","Invalid").toString().toStdString();
  }
  s.endArray(); // DataTiles
  int MaskCASSID(s.value("MaskCASSID",31).toInt());
  size = s.beginReadArray("MaskTiles");
  machineVals_t masktilenames;
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    masktilenames[i] = s.value("HDF5Key","Invalid").toString().toStdString();
  }
  s.endArray(); // MaskTiles
  int GainCASSID(s.value("GainCASSID",32).toInt());
  size = s.beginReadArray("GainTiles");
  machineVals_t gaintilenames;
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    gaintilenames[i] = s.value("HDF5Key","Invalid").toString().toStdString();
  }
  s.endArray(); // GainTiles
  string cellIdKey(s.value("CellId_HDF5Key","Invalid").toString().toStdString());
  string trainIdKey(s.value("TrainId_HDF5Key","Invalid").toString().toStdString());
  string pulseIdKey(s.value("PulseId_HDF5Key","Invalid").toString().toStdString());
  s.endGroup(); // AGPID
  s.endGroup(); // XFEHDF5FileInput

  _status = lmf::PausableThread::running;
  Tokenizer tokenize;

  /** retrieve all lines from the file */
  Log::add(Log::VERBOSEINFO,"XFELHDF5FileInput::run(): try to open filelist '" +
           _filelistname + "'");
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("XFELHDF5FileInput::run(): filelist '" + _filelistname +
                           "' could not be opened");
  vector<string> filelist(tokenize(filelistfile));
  filelistfile.close();

  /** go through the list of files */
  vector<string>::const_iterator filelistIt(filelist.begin());
  vector<string>::const_iterator filelistEnd(filelist.end());

  typedef vector<AGIPDTile> fileset_t;
  typedef vector<fileset_t> filesetlist_t;
  filesetlist_t filesetlist;
  for (;(filelistIt != filelistEnd); ++filelistIt)
  {
    /** the line consists of the filenames that all belong to the same pulse
     *  trains so put them into a map.
     */
    std::stringstream ss(*filelistIt);
    string filepath;
    filesetlist.resize(filesetlist.size()+1);
    fileset_t &fs(filesetlist.back());
    /** parse the tile information into the tile */
    while (getline(ss,filepath,','))
    {
      /** create a new tile */
      AGIPDTile tile;
      /** find out which tile this file is used for */
      size_t afterTileID(filepath.find_last_of('-'));
      tile.id = atoi(filepath.substr(afterTileID-2,2).c_str());
      /** open the file and add the handle for the file to the tile */
      tile.fh = AGIPDTile::h5handle_t(new hdf5::Handler(filepath,"r"));
      /** defer the datasetname from the id */
      tile.dataDsetName = datatilenames[tile.id];
      /** defer the datasetname from the id */
      tile.maskDsetName = masktilenames[tile.id];
      /** defer the datasetname from the id */
      tile.gainDsetName = gaintilenames[tile.id];
      /** now add the tile to the fileset */
      fs.push_back(tile);
    }
  }
  /** now go through all set of files */
  for (size_t i(0); (!shouldQuit()) && (i<filesetlist.size());++i)
  {
    fileset_t& fileset(filesetlist[i]);

    /** get the shape from the first datafile (should be the same for all data
     *  files). The first dimension is the number of images, so we iterate over
     *  this number
     */
    const hdf5::shape_t shape(fileset[0].fh->shape(fileset[0].dataDsetName));
    const size_t nCols(shape[shape.size()-1]);
    const size_t nRows(shape[shape.size()-2]);
    const size_t nTiles(fileset.size());
    _totalevents = shape[0];
    fileset_t::iterator fsit(fileset.begin());
    while(fsit < fileset.end())
    {
      fsit->nCols = nCols;
      fsit->nRows = nRows;
      fsit->size = (nCols*nRows);
      ++fsit;
    }
    /** pre cache the data */
    for_each(fileset.begin(),fileset.end(),preCacheData);

    /** get all the trainids and the pulseids from the first datafile, they
     *  later used to compile a unique eventid
     */
    vector<uint64_t> trainIds;
    size_t length;
    fileset[0].fh->readArray(trainIds,length,trainIdKey);

    vector<uint64_t> pulseIds;
    fileset[0].fh->readArray(pulseIds,length,pulseIdKey);

    vector<uint16_t> cellIds;
    fileset[0].fh->readArray(cellIds,length,cellIdKey);

    //for (size_t i(0); (!shouldQuit()) && (i < shape[0]); i+=2)
    //for (size_t i(0); (!shouldQuit()) && (i < shape[0]);++i)
    for (size_t iTrain(0); (!shouldQuit()) && (iTrain < 250);iTrain+=4)
    {
      for (size_t iBunch(4); (!shouldQuit()) && (iBunch < 33);iBunch+=4)
      {
        /** flag that tell whether the data is good */
        bool isGood(true);
        /** retrieve a new element from the ringbuffer. If one didn't get a
       *  an element (when the end iterator of the buffer is returned).
       *  Continue to the next iteration, where it is checked if the thread
       *  should quit.
       */
        rbItem_t rbItem(getNextFillable());
        if (rbItem == _ringbuffer.end())
          continue;
        CASSEvent& evt(*rbItem->element);

        /** create the event id from the trainid and the pulseid */
        evt.id() = ((trainIds[i] & 0x0000FFFFFFFFFFFF)<< 16) +
            ((pulseIds[i] & 0x000000000000FFFF));

        /** get reference to all devices of the CASSEvent and an iterator*/
        CASSEvent::devices_t &devices(evt.devices());
        CASSEvent::devices_t::iterator devIt;
        /** retrieve the pixel detector part of the cassevent */
        devIt = devices.find(CASSEvent::PixelDetectors);
        if(devIt == devices.end())
          throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
                              " contain a pixeldetector device");
        pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
        /** retrieve the right detector from the cassevent and reset it*/
        pixeldetector::Detector &data(pixdev.dets()[DataCASSID]);
        pixeldetector::Detector::frame_t &dataframe(data.frame());
        dataframe.clear();
        dataframe.resize(nRows*nCols*nTiles);
        /** copy the det data to the frame */
        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyDataTileToFrame,_1,
        //                                                       dataframe.begin(),i));
        for_each(fileset.begin(),fileset.end(),tr1::bind(copyCorImageFromCacheToFrame,_1,
                                                         dataframe.begin(),i));
        /** set the detector parameters and add the event id */
        data.columns() = nCols;
        data.rows() = nRows*nTiles;
        data.id() = evt.id();
        /** retrieve the right detector from the cassevent and reset it*/
        pixeldetector::Detector &gain(pixdev.dets()[GainCASSID]);
        pixeldetector::Detector::frame_t &gainframe(gain.frame());
        gainframe.clear();
        gainframe.resize(nRows*nCols*nTiles);
        /** copy the det data to the frame */
        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyGainTileToFrame,_1,
        //                                                       gainframe.begin(),i));
        for_each(fileset.begin(),fileset.end(),tr1::bind(copyGainFromCacheToFrame,_1,
                                                         gainframe.begin(),i));
        /** set the detector parameters and add the event id */
        gain.columns() = nCols;
        gain.rows() = nRows*nTiles;
        gain.id() = evt.id();
        /** retrieve the right detector to hold the mask from the cassevent and reset it*/
        pixeldetector::Detector &mask(pixdev.dets()[MaskCASSID]);
        pixeldetector::Detector::frame_t &maskframe(mask.frame());
        maskframe.clear();
        maskframe.resize(nRows*nCols*nTiles);
        /** copy the det data to the frame */
        //      for_each(fileset.begin(),fileset.end(),tr1::bind(copyTileToMask,_1,
        //                                                       maskframe.begin(),i));
        for_each(fileset.begin(),fileset.end(),tr1::bind(copyMaskFromCacheToFrame,_1,
                                                         maskframe.begin(),i));
        /** set the detector parameters and add the event id */
        mask.columns() = nCols;
        mask.rows() = nRows*nTiles;
        mask.id() = evt.id();

        /** retrieve the machine detector part of the cassevent */
        devIt = devices.find(CASSEvent::MachineData);
        if(devIt == devices.end())
          throw runtime_error(string("xfelhdf5fileinput: CASSEvent does not") +
                              " contain a machinedata device");
        MachineData::Device &md (dynamic_cast<MachineData::Device&>(*(devIt->second)));
        md.BeamlineData()["CellId"] = cellIds[i];
        md.BeamlineData()["PulseId"] = pulseIds[i];
        md.BeamlineData()["TrainId"] = trainIds[i];

        /** tell the ringbuffer that we're done with the event */
        newEventAdded(0);
        ++_counter;
        _ringbuffer.doneFilling(rbItem, isGood);
      }
    }
  }

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


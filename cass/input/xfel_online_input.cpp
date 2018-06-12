// Copyright (C) 2018 Lutz Foucar

/**
 * @file xfel_online_input.cpp contains input that the xfel api
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "xfel_online_input.h"

#include "cass_settings.h"
#include "cass_exceptions.hpp"
#include "log.h"

#include "acqiris_device.hpp"
#include "pixeldetector.hpp"
#include "machine_device.hpp"
#include "kb_client.hpp"

using namespace cass;
using namespace std;


void XFELOnlineInput::instance(RingBuffer<CASSEvent> &buffer,
                               Ratemeter &ratemeter,
                               Ratemeter &loadmeter,
                               bool quitwhendone,
                               QObject *parent)
{
  if(_instance)
    throw logic_error("XFELOnlineInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new XFELOnlineInput(buffer,ratemeter,loadmeter,quitwhendone,parent));
}

XFELOnlineInput::XFELOnlineInput(RingBuffer<CASSEvent> &ringbuffer,
                                 Ratemeter &ratemeter,
                                 Ratemeter &loadmeter,
                                 bool quitwhendone,
                                 QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitwhendone),
    _counter(0),
    _scounter(0)
{}



void XFELOnlineInput::runthis()
{
  _status = lmf::PausableThread::running;

  CASSSettings s;
  s.beginGroup("XFELOnlineInput");
  /** info specific to the xfel data server */
  string serverAddress(s.value("ServerAddress","tcp://localhost:1234").toString().toStdString());
  /** info where the nbrPulses is within the transferred data */
  string nPulsesPath(s.value("PathToNbrPulsesInTrain","header.pulseCount").toString().toStdString());

  /** info about the source of the data of interest */
  string source(s.value("Source","SPB_DET_AGIPD1M-1/DET/detector-1").toString().toStdString());
  /** info where the image data is within the transferred data */
  string imageDataPath(s.value("PathToImage","image.data").toString().toStdString());
  /** the id that the data should have within the cass-event */
  int det_CASSID(s.value("CASSID",30).toInt());
  s.endGroup(); //XFELOnlineInput

  /** create a karabo client that allows to connect to online karabo */
  karabo_bridge::Client client;

  /** write the data structure that is sent by the server to the log */
  Log::add(Log::INFO,"XFELOnlineInput::run(): trying to connect to server at '"+
                     serverAddress + "'");

  /** connect to the xfel client */
  client.connect(serverAddress);

  /** write the data structure that is sent by the server to the log */
  Log::add(Log::INFO,"XFELOnlineInput::run(): connected to server at '"+
                     serverAddress + "'. Now waiting for data.");

  /** write the data structure that is sent by the server to the log */
  Log::add(Log::INFO,"XFELOnlineInput::run(): available data from the server:\n"+
                     client.showNext());

  /** run until the thread is told to quit */
  Log::add(Log::INFO,"XFELOnlineInput::run(): starting loop");

  while(!shouldQuit())
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** now retrive new data from the socket */
    auto data(client.next());

    /** get the info about the number of pulses in the train */
    //auto nPulses(data[source][nPulsesPath].as<vector<uint64_t> >());
    //auto nPulsesShape(data[source][nPulsesPath].get().via.array.size);
    size_t nPulses(64);

    /** get the shape of the detector (encodes the pulses in the train and the
     *  and the shape itself)
     *
     *  should be in the shape of nPulses,nModules,512,128 but is most likely in the
     *  shape of nModules,128,512,nPulses so one needs to transpose the axis.
     */
    const auto det_shape(data[source].array[imageDataPath].shape());
    const bool dataNeedsPermutation(det_shape[3] != 128);
    size_t nPulsesFromImage;
    size_t nModules;
    size_t nRowsInModule;
    size_t nCols;
    if (dataNeedsPermutation)
    {
      // if in nModules,128,512,nPulses
      nPulsesFromImage = (det_shape[3]);
      nModules = (det_shape[0]);
      nRowsInModule = (det_shape[2]);
      nCols = (det_shape[1]);
    }
    else
    {
      // if in nPulses,nModules,512,128
      nPulsesFromImage = (det_shape[0]);
      nModules = (det_shape[1]);
      nRowsInModule = (det_shape[2]);
      nCols = (det_shape[3]);
    }
    const size_t sizeofOneDet(nModules*nRowsInModule*nCols);
    const size_t nCASSRows(nModules*nRowsInModule);

    /** get the detector data */
    pixeldetector::Detector::frame_t det_data;
    if (data[source].array[imageDataPath].dtype() == "uint16_t")
    {
      const auto tmp(data[source].array[imageDataPath].as<uint16_t>());
      det_data.assign(tmp.begin(),tmp.end());
    }
    else if (data[source].array[imageDataPath].dtype() == "float32")
    {
      //auto tmp(data[source].array[imageDataPath].as<float>());
      //det_data.assign(tmp.begin(),tmp.end());
      const auto ptr(data[source].array[imageDataPath].data<float>());
      const auto nElements(data[source].array[imageDataPath].size());
      if (dataNeedsPermutation)
      {
        // permute the axis of the original data to go with the exspected layout
        det_data.resize(nElements);
        for (size_t iModule(0); iModule < nModules ; ++iModule)
        {
          for (size_t iColumn(0); iColumn < nCols ; ++iColumn)
          {
            for (size_t iRow(0); iRow < nRowsInModule ; ++iRow)
            {
              for (size_t iPulse(0); iPulse < nPulsesFromImage; ++iPulse)
              {
                auto origIDX(iPulse +
                             iRow*nPulses +
                             iColumn*nPulsesFromImage*nRowsInModule +
                             iModule*nPulsesFromImage*nRowsInModule*nCols);
                auto goodIDX(iRow +
                             iColumn*nRowsInModule +
                             iModule*nRowsInModule*nCols +
                             iPulse*nRowsInModule*nCols*nModules);
                det_data[goodIDX] = ptr[origIDX];
              }
            }
          }
        }
      }
      else
      {
        det_data.assign(ptr,ptr+nElements);
      }
    }

    /** check if the data is consistent */
    if (nPulses != nPulsesFromImage)
    {
      Log::add(Log::ERROR,string("The number of pulses within the header '") +
                          toString(nPulses) + "' and the detector data '" +
                          toString(nPulsesFromImage) + "' mismatch. "+
                          "Skipping train.");
    }

    /** go through all pulses in the train */
    for(size_t pulseID(0); pulseID < nPulses; ++pulseID)
    {
      /** retrieve a new element from the ringbuffer, continue with next iteration
       *  in case the retrieved element is the iterator to the last element of the
       *  buffer.
       */
      rbItem_t rbItem(getNextFillable());
      if (rbItem == _ringbuffer.end())
        continue;
      CASSEvent &evt(*rbItem->element);
      evt.id() = _counter;

      /** get reference to all devices of the CASSEvent and an iterator*/
      CASSEvent::devices_t &devices(evt.devices());
      CASSEvent::devices_t::iterator devIt;

      /** retrieve the pixel detector part of the cassevent */
      devIt = devices.find(CASSEvent::PixelDetectors);
      if(devIt == devices.end())
        throw runtime_error(string("XFELOnlineInput: CASSEvent does not ") +
                                   "contain a pixeldetector device");
      pixeldetector::Device &pixdev(dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
      /** retrieve the right detector from the cassevent and reset it*/
      pixeldetector::Detector &det(pixdev.dets()[det_CASSID]);
      det.frame().clear();
      det.columns() = nCols;
      det.rows() = nCASSRows;
      det.id() = _counter;

      /** copy the data of the pulse from the xfel input to the cassevent */
      auto offsetToBegin(pulseID * sizeofOneDet);
      auto offsetToEnd((pulseID+1) * sizeofOneDet);
      det.frame().assign(det_data.begin()+offsetToBegin,det_data.begin()+offsetToEnd);

      /** tell the ringbuffer that we're done with the event */
      ++_counter;
      _ringbuffer.doneFilling(rbItem, 1);
    }// done going through all pulses in the train
    newEventAdded(data.size());
  }
  Log::add(Log::INFO,"XFELOnlineInput::run(): Quitting loop");
}

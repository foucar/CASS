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
  string serverAddress(s.value("ServerAddress","tcp://53.104.0.52:10000").toString().toStdString());
  /** info where the data is within the transferred data */
  string imageDataPath(s.value("PathToImage","data.data").toString().toStdString());
  /** the id that the data should have within the cass-event */
  int det_CASSID(s.value("CASSID",30).toInt());
  s.endGroup(); //XFELOnlineInput

  /** create a karabo client that allows to connect to online karabo */
  karabo_bridge::Client client;

  /** connect to the xfel client */
  client.connect(serverAddress);

  /** run until the thread is told to quit */
  Log::add(Log::DEBUG0,"XFELOnlineInput::run(): starting loop");

  while(!shouldQuit())
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** now retrive new data from the socket */
    karabo_bridge::kb_data data(client.next());

    /** get the detector data */
    vector<uint16_t> det_data = data.array[imageDataPath].as<uint16_t>();

    /** get the shape of the detector (encodes the pulses in the train and the
     *  and the shape itself
     */
    const vector<int> det_shape(data.array[imageDataPath].shape());
    const int nPulses(det_shape[0]);
    const uint16_t nModules(det_shape[1]);
    const uint16_t nRowsInModule(det_shape[2]);
    const uint16_t nCASSRows(nModules*nRowsInModule);
    const uint16_t nCols(det_shape[3]);
    const size_t sizeofOneDet(nModules*nRowsInModule*nCols);

    /** go through all pulses in the train */
    for(int pulseID(0); pulseID < nPulses; ++pulseID)
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
      det.frame().resize(sizeofOneDet);
      det.columns() = nCASSRows;
      det.rows() = nCols;
      det.id() = _counter;

      /** get pointer to the xfel detector data and advance it to the right
       *  pulse within the train
       */
      vector<uint16_t>::const_iterator detIt(det_data.begin());
      advance(detIt,pulseID*sizeofOneDet);

     /** copy the data from the xfel input to the cassevent */
     copy(detIt,detIt+sizeofOneDet,det.frame().begin());

      /** tell the ringbuffer that we're done with the event */
      ++_counter;
      _ringbuffer.doneFilling(rbItem, 1);
    }// done going through all pulses in the train
    newEventAdded(0);
  }
  Log::add(Log::INFO,"XFELOnlineInput::run(): Quitting loop");
}
